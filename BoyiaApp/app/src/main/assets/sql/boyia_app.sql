DROP TABLE IF EXISTS downloads;

CREATE TABLE downloads (
    id integer primary key autoincrement,
    file_name varchar(200) not null,
    file_url varchar(200) not null,
    file_path varchar(200) not null,
    current_size integer,
    max_len integer,
    status integer
);