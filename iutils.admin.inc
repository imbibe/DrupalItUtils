<?php

function iutils_admin_structure_form($form, $form_state) {
	$form['download_db_backup'] = array(
		'#type' => 'button',
		'#value' => t('Download DB backup'),
		'#submit' => array('iutils_admin_structure_form_download_db_backup'),
		'#executes_submit_callback' => TRUE,
	);
	return ($form);
}

function iutils_admin_structure_form_download_db_backup ($form, &$form_state) {
	//http://davidwalsh.name/backup-mysql-database-php
	global $databases;
	
	//10 minutes
	set_time_limit(600);
	while(ob_get_level() > 0) {
		ob_end_clean ();
	}

	$dbhost = $databases['default']['default']['host'];
	$dbname = $databases['default']['default']['database'];
	$dbuser = $databases['default']['default']['username'];
	$dbpass = $databases['default']['default']['password'];

	/*$temp_file = tempnam(sys_get_temp_dir(), 'tmp');
	$command = "mysqldump -h $dbhost -u $dbuser -p$dbpass $dbname > $temp_file";
	system($command);*/
	$temp_file = _iutils_backup_tables($dbhost, $dbname, $dbuser, $dbpass);

	$compressed_file = _iutils_gzCompressFile($temp_file);
	unlink($temp_file);
	
	$attachment_name = $dbname . date("-Y-m-d H-i-s") . '.sql.gz';

	header('Content-Description: File Transfer');
	header('Content-Type: application/x-gzip');
	//header('Content-Disposition: attachment; filename=' . basename($file));
	header('Content-Disposition: attachment; filename=' . $attachment_name);
	header('Expires: 0');
	header('Cache-Control: must-revalidate');
	header('Pragma: public');
	header('Content-Length: ' . filesize($compressed_file));
	readfile($compressed_file);

	unlink($compressed_file);
	exit;
}

function _iutils_backup_tables($host, $name, $user, $pass, $tables = '*') {
	$link = mysql_connect($host, $user, $pass);
	mysql_select_db($name, $link);
	
	//get all of the tables
	if($tables == '*') {
		$tables = array();
		$result = mysql_query('SHOW TABLES');
		while($row = mysql_fetch_row($result)) {
			$tables[] = $row[0];
		}
	} else {
		$tables = is_array($tables) ? $tables : explode(',', $tables);
	}
	
	$temp_file = tempnam(sys_get_temp_dir(), 'tmp');	
	$handle = fopen($temp_file, 'w+');
	
	$content = "";

	//cycle through
	foreach($tables as $table) {
		_iutils_write_buffered($handle, $content, 'DROP TABLE ' . $table . ';');
		
		$result = mysql_query('SHOW CREATE TABLE ' . $table);
		$row2 = mysql_fetch_row($result);
		mysql_free_result($result);
		
		_iutils_write_buffered($handle, $content, "\n\n" . $row2[1] . ";\n\n");
		
		$result = mysql_query('SELECT * FROM ' . $table);
		$num_fields = mysql_num_fields($result);
		
		for ($i = 0; $i < $num_fields; $i++) {
			while($row = mysql_fetch_row($result)) {
				_iutils_write_buffered($handle, $content, 'INSERT INTO '. $table .' VALUES(');
				for($j=0; $j<$num_fields; $j++) {
					$row[$j] = addslashes($row[$j]);
					$row[$j] = ereg_replace("\n", "\\n", $row[$j]);
					if (isset($row[$j])) { _iutils_write_buffered($handle, $content, '"'.$row[$j].'"') ; } else { _iutils_write_buffered($handle, $content, '""'); }
					if ($j < ($num_fields-1)) { _iutils_write_buffered($handle, $content, ','); }
				}
				
				_iutils_write_buffered($handle, $content, ");\n");
			}
		}
		
		_iutils_write_buffered($handle, $content, "\n\n\n");
		
		mysql_free_result($result);
	}
		
	fclose($handle);
	
	return ($temp_file);
}

function _iutils_write_buffered($handle, &$existing, $new) {
	$existing .= $new;
	
	if(strlen($existing) > 128 *1024) {
		fwrite($handle, $existing);
		$existing = "";
	}
}

/**
 * GZIPs a file on disk (appending .gz to the name)
 *
 * From http://stackoverflow.com/questions/6073397/how-do-you-create-a-gz-file-using-php
 * Based on function by Kioob at:
 * http://www.php.net/manual/en/function.gzwrite.php#34955
 * 
 * @param string $source Path to file that should be compressed
 * @param integer $level GZIP compression level (default: 9)
 * @return string New filename (with .gz appended) if success, or false if operation fails
 */
function _iutils_gzCompressFile($source, $level = 9){ 
    $dest = $source . '.gz'; 
    $mode = 'wb' . $level;
	
    $error = false; 
    if ($fp_out = gzopen($dest, $mode)) { 
        if ($fp_in = fopen($source,'rb')) { 
            while (!feof($fp_in)) {
                gzwrite($fp_out, fread($fp_in, 1024 * 512)); 
			}
            fclose($fp_in); 
        } else {
            $error = true; 
        }
        gzclose($fp_out); 
    } else {
        $error = true; 
    }
	
    if ($error) {
        return false; 
    } else {
        return $dest; 
	}
}
