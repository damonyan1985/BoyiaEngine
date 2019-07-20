cmake_minimum_required(VERSION 3.4.1)

project(ssl)

add_definitions(-DOPENSSL_THREADS -D_REENTRANT -DDSO_DLFCN -DHAVE_DLFCN_H -DL_ENDIAN)

add_definitions(-DOPENSSL_NO_CAMELLIA -DOPENSSL_NO_CAPIENG -DOPENSSL_NO_CAST -DOPENSSL_NO_CMS -DOPENSSL_NO_GMP -DOPENSSL_NO_IDEA -DOPENSSL_NO_JPAKE -DOPENSSL_NO_MD2 -DOPENSSL_NO_MDC2 -DOPENSSL_NO_RC5 -DOPENSSL_NO_SHA0 -DOPENSSL_NO_RFC3779 -DOPENSSL_NO_SEED -DOPENSSL_NO_STORE -DOPENSSL_NO_WHIRLPOOL)

add_definitions(-DOPENSSL_NO_HW -DOPENSSL_NO_ENGINE -DZLIB)

#add_definitions(-DNO_WINDOWS_BRAINDEATH)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wno-unused-parameter")

set(OPENSSL_PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../")

set(
SSL_SRC_LIST
	s2_meth.c
	s2_srvr.c
	s2_clnt.c
	s2_lib.c
	s2_enc.c
	s2_pkt.c
	s3_meth.c
	s3_srvr.c
	s3_clnt.c
	s3_lib.c
	s3_enc.c
	s3_pkt.c
	s3_both.c
	s23_meth.c
	s23_srvr.c
	s23_clnt.c
	s23_lib.c
	s23_pkt.c
	t1_meth.c
	t1_srvr.c
	t1_clnt.c
	t1_lib.c
	t1_enc.c
	t1_reneg.c
	ssl_lib.c
	ssl_err2.c
	ssl_cert.c
	ssl_sess.c
	ssl_ciph.c
	ssl_stat.c
	ssl_rsa.c
	ssl_asn1.c
	ssl_txt.c
	ssl_algs.c
	bio_ssl.c
	ssl_err.c
	kssl.c
)

add_library(
    ssl
    SHARED
    ${SSL_SRC_LIST}
)

target_include_directories(
    ssl
    PRIVATE
    ${OPENSSL_PROJECT_PATH}
    ${OPENSSL_PROJECT_PATH}/ssl
    ${OPENSSL_PROJECT_PATH}/crypto
    ${OPENSSL_PROJECT_PATH}/include
    )

target_link_libraries(
    ssl
    crypto
)