package com.boyia.app.client;

import java.net.InetAddress;

import io.netty.bootstrap.Bootstrap;
import io.netty.channel.Channel;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.nio.NioSocketChannel;

public class BoyiaClient {
    private final static String BOYIA_CLIENT_IP = "192.168.0.100";
    private final int BOYIA_CLIENT_PORT = 60001;

    private Channel mChannel;
    public BoyiaClient() {
        try {
            EventLoopGroup group = new NioEventLoopGroup();
            Bootstrap bootstrap = new Bootstrap();
            bootstrap.group(group)
                    .channel(NioSocketChannel.class)
                    .handler(new BoyiaClientInitializer());

            mChannel = bootstrap.connect(InetAddress.getByName(BOYIA_CLIENT_IP), BOYIA_CLIENT_PORT).sync().channel();
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    public void sendMessage(String msg) {
        if (mChannel != null) {
            mChannel.writeAndFlush(msg);
        }
    }
}
