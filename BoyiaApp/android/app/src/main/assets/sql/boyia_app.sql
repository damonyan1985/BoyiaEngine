DROP TABLE IF EXISTS downloads;

CREATE TABLE downloads (
    id integer primary key autoincrement,
    file_name varchar(200) not null,
    file_url varchar(200) not null,
    file_path varchar(200) not null,
    current_size integer,
    max_len integer,
    file_md5 varchar(200),
    status integer
);

UPDATE sqlite_sequence SET seq = 10000 WHERE name = 'downloads';