package com.boyia.app.loader.http;

import android.content.res.AssetManager;

import com.boyia.app.common.BaseApplication;

import java.io.IOException;
import java.io.InputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.security.KeyStore;
import java.security.SecureRandom;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;

public class SSLHelper {
    private static final String CERT_PATH = "cert";
    private static final String KEY_STORE_PASSWORD = "";
    private static final String KEY_STORE_TRUST_PASSWORD = "";

    public static SSLInfo getSSLInfo() {
        return new SSLInfo();
    }

    public static TrustManagerFactory getCertificates() {
        try {
            AssetManager assets = BaseApplication.getInstance().getAssets();
            String certs[] = assets.list(CERT_PATH);
            if (certs == null || certs.length == 0) {
                return null;
            }

            KeyStore keyStore = KeyStore.getInstance(KeyStore.getDefaultType());
            keyStore.load(null);
            CertificateFactory factory = CertificateFactory.getInstance("X.509");
            for (int i = 0; i < certs.length; ) {
                InputStream input = assets.open(certs[i]);
                keyStore.setCertificateEntry(Integer.toString(i++), factory.generateCertificate(input));
                input.close();
            }

            TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
            tmf.init(keyStore);
            return tmf;
        } catch (Exception ex) {
            ex.printStackTrace();
        }
        return null;
    }

    public static class SSLInfo {
        SSLSocketFactory mFactory;
        X509TrustManager mTrustManager;

        public SSLInfo() {
            try {
                SSLContext context = SSLContext.getInstance("TLS");
                TrustManagerFactory factory = getCertificates();
                mTrustManager = factory != null ? (X509TrustManager) factory.getTrustManagers()[0] :  new X509TrustManager() {
                    @Override
                    public void checkClientTrusted(X509Certificate[] chain, String authType) throws CertificateException {
                    }

                    @Override
                    public void checkServerTrusted(X509Certificate[] chain, String authType) throws CertificateException {
                    }

                    // 返回授信证书列表
                    @Override
                    public X509Certificate[] getAcceptedIssuers() {
                        //return new X509Certificate[0];
                        return new X509Certificate[]{};
                    }
                };
                context.init(
                        null,
                        factory != null ? factory.getTrustManagers() : new TrustManager[] { mTrustManager },
                        new SecureRandom()
                );

                //return context.getSocketFactory();
                mFactory =  new TlsCompatSocketFactory(context.getSocketFactory());
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    }

    public static class TlsCompatSocketFactory extends SSLSocketFactory {
        private static final String[] TLS_VERSION_LIST = {"TLSv1", "TLSv1.1", "TLSv1.2"};

        final SSLSocketFactory mTarget;

        public TlsCompatSocketFactory(SSLSocketFactory target) {
            mTarget = target;
        }

        @Override
        public String[] getDefaultCipherSuites() {
            return mTarget.getDefaultCipherSuites();
        }

        @Override
        public String[] getSupportedCipherSuites() {
            return mTarget.getSupportedCipherSuites();
        }

        @Override
        public Socket createSocket(Socket s, String host, int port, boolean autoClose) throws IOException {
            return supportTLS(mTarget.createSocket(s, host, port, autoClose));
        }

        @Override
        public Socket createSocket(String host, int port) throws IOException {
            return supportTLS(mTarget.createSocket(host, port));
        }

        @Override
        public Socket createSocket(String host, int port, InetAddress localHost, int localPort) throws IOException {
            return supportTLS(mTarget.createSocket(host, port, localHost, localPort));
        }

        @Override
        public Socket createSocket(InetAddress host, int port) throws IOException {
            return supportTLS(mTarget.createSocket(host, port));
        }

        @Override
        public Socket createSocket(InetAddress address, int port, InetAddress localAddress, int localPort) throws IOException {
            return supportTLS(mTarget.createSocket(address, port, localAddress, localPort));
        }

        @Override
        public Socket createSocket() throws IOException {
            return supportTLS(mTarget.createSocket());
        }

        private Socket supportTLS(Socket s) {
            if (s instanceof SSLSocket) {
                ((SSLSocket) s).setEnabledProtocols(TLS_VERSION_LIST);
            }
            return s;
        }
    }
}
