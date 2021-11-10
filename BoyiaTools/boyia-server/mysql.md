# Mysql配置
## 安装
1. 密码设置
```
忽略初始密码
./mysqld_safe --skip-grant-tables &
```
```
设置初始密码
mysql>use mysql;
mysql>flush privileges; 
mysql>alter user 'root'@'localhost' identified by '123456';
mysql>quit;
```
2. 服务启动
```
mysql.server start 
mysql.server restart 
```