mysqldump --opt -u [uname] -p[pass] [dbname] > [backupfile.sql]

mysqldump -u root -p  db_name > db_name.sql
mysqldump -u root -p --all-databases > alldb_backup.sql


mysqldump -u [uname] -p[pass] [dbname] | gzip -9 > [backupfile.sql.gz]

gunzip [backupfile.sql.gz]

mysql -u [uname] -p[pass] [db_to_restore] < [backupfile.sql]

mysql -u root -p db_name < db_name.sql

gunzip < [backupfile.sql.gz] | mysql -u [uname] -p[pass] [dbname]

mysqlimport -u [uname] -p[pass] [dbname] [backupfile.sql]



Backup all MySQL Databases to individual files: (http://www.commandlinefu.com/commands/view/2916/backup-all-mysql-databases-to-individual-files)

for I in $(mysql -e 'show databases' -s --skip-column-names); do mysqldump $I | gzip > "$I.sql.gz"; done

