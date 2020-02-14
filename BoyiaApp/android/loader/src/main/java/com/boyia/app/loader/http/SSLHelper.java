package com.boyia.app.loader.http;

import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.security.SecureRandom;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

public class SSLHelper {
    public static SSLInfo getSSLInfo() {
        return new SSLInfo();
    }

    public static class SSLInfo {
        SSLSocketFactory mFactory;
        X509TrustManager mTrustManager;

        public SSLInfo() {
            try {
                SSLContext context = SSLContext.getInstance("TLS");
                mTrustManager = new X509TrustManager() {
                    @Override
                    public void checkClientTrusted(X509Certificate[] chain, String authType) throws CertificateException {
                    }

                    @Override
                    public void checkServerTrusted(X509Certificate[] chain, String authType) throws CertificateException {
                    }

                    @Override
                    public X509Certificate[] getAcceptedIssuers() {
                        //return new X509Certificate[0];
                        return new X509Certificate[]{};
                    }
                };
                context.init(null, new TrustManager[] { mTrustManager }, new SecureRandom());

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
