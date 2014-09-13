<?php

function it_utils_admin_structure_form($form, $form_state) {
	$form['download_db_backup'] = array(
		'#type' => 'button',
		'#value' => t('Download DB backup'),
		'#submit' => array('it_utils_admin_structure_form_download_db_backup'),
		'#executes_submit_callback' => TRUE,
	);

	$form['download_public_files_backup'] = array(
		'#type' => 'button',
		'#value' => t('Download Public Files backup'),
		'#submit' => array('it_utils_admin_structure_form_download_public_files_backup'),
		'#executes_submit_callback' => TRUE,
	);

	return ($form);
}

//Database backup
function it_utils_admin_structure_form_download_db_backup ($form, &$form_state) {
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
	$temp_file = _it_utils_backup_tables($dbhost, $dbname, $dbuser, $dbpass);

	$compressed_file = _it_utils_gzCompressFile($temp_file);
	unlink($temp_file);
	
	$attachment_name = $dbname . date("-Y-m-d-H-i-s") . '.sql.gz';

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

function _it_utils_backup_tables($host, $name, $user, $pass, $tables = '*') {
	//http://davidwalsh.name/backup-mysql-database-php
	$link = mysqli_connect($host, $user, $pass, $name);
	
	//get all of the tables
	if($tables == '*') {
		$tables = array();
		$result = mysqli_query($link, 'SHOW TABLES');
		while($row = mysqli_fetch_row($result)) {
			$tables[] = $row[0];
		}
		mysqli_free_result($result);
	} else {
		$tables = is_array($tables) ? $tables : explode(',', $tables);
	}
	
	$temp_file = tempnam(sys_get_temp_dir(), 'tmp');	
	$handle = fopen($temp_file, 'w+');
	
	$content = "";

	//cycle through
	foreach($tables as $table) {
		_it_utils_write_buffered($handle, $content, 'DROP TABLE ' . $table . ';');
		
		$result = mysqli_query($link, 'SHOW CREATE TABLE ' . $table);
		$row2 = mysqli_fetch_row($result);
		mysqli_free_result($result);
		
		_it_utils_write_buffered($handle, $content, "\n\n" . $row2[1] . ";\n\n");
		
		$result = mysqli_query($link, 'SELECT * FROM ' . $table);
		$num_fields = mysqli_num_fields($result);
		
		for ($i = 0; $i < $num_fields; $i++) {
			while($row = mysqli_fetch_row($result)) {
				_it_utils_write_buffered($handle, $content, 'INSERT INTO '. $table .' VALUES(');
				for($j=0; $j<$num_fields; $j++) {
					$row[$j] = addslashes($row[$j]);
					$row[$j] = preg_replace("/\n/", "\\n", $row[$j]);
					if (isset($row[$j])) { _it_utils_write_buffered($handle, $content, '"'.$row[$j].'"') ; } else { _it_utils_write_buffered($handle, $content, '""'); }
					if ($j < ($num_fields-1)) { _it_utils_write_buffered($handle, $content, ','); }
				}
				
				_it_utils_write_buffered($handle, $content, ");\n");
			}
		}
		
		_it_utils_write_buffered($handle, $content, "\n\n\n");
		
		mysqli_free_result($result);
	}
		
	mysqli_close($link);
	
	_it_utils_write_buffered($handle, $content, "", TRUE);
	
	fclose($handle);
	
	return ($temp_file);
}

function _it_utils_write_buffered($handle, &$existing, $new = "", $force = FALSE) {
	$existing .= $new;
	
	if((strlen($existing) > 128 *1024) || $force) {
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
function _it_utils_gzCompressFile($source, $level = 9){ 
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



//Public Files backup
function it_utils_admin_structure_form_download_public_files_backup ($form, &$form_state) {
	//10 minutes
	set_time_limit(600);
	while(ob_get_level() > 0) {
		ob_end_clean ();
	}
	
	$publicFilesDir = variable_get('file_public_path', conf_path() . '/files');
	
	$temp_file = tempnam(sys_get_temp_dir(), 'tmp');	

	$za = new FlxZipArchive;
	$res = $za->open($temp_file, ZipArchive::CREATE);
	if($res === TRUE) {
		$za->addDir($publicFilesDir, basename($publicFilesDir));
		$za->close();
	} else{
		echo 'Could not create a zip archive';
	}

	header("Pragma: public");
	header("Expires: 0");
	header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
	header("Cache-Control: private", false);
	header("Content-Type: application/zip");
	header("Content-Disposition: attachment; filename=" . basename($publicFilesDir) . ".zip;" );
	header("Content-Transfer-Encoding: binary");
	header("Content-Length: " . filesize($temp_file));
	readfile($temp_file);

	unlink($temp_file);
	exit;
}

//http://stackoverflow.com/questions/17708562/zip-all-files-in-directory-and-download-zip-generated
class FlxZipArchive extends ZipArchive {
    /** Add a Dir with Files and Subdirs to the archive
		@param string $location Real Location
		@param string $name Name in Archive
		@author Nicolas Heimann
		@access private  
	**/
    public function addDir($location, $name) {
        $this->addEmptyDir($name);

        $this->addDirDo($location, $name);
     }

    /**  Add Files & Dirs to archive
		@param string $location Real Location
		@param string $name Name in Archive
		@author Nicolas Heimann
		@access private
	**/
    private function addDirDo($location, $name) {
        $name .= '/';
        $location .= '/';

        // Read all Files in Dir
        $dir = opendir ($location);
        while ($file = readdir($dir)) {
            if ($file == '.' || $file == '..') continue;
			
            // Recursive, If dir: FlxZipArchive::addDir(), else ::File();
            $do = (filetype($location . $file) == 'dir') ? 'addDir' : 'addFile';
            $this->$do($location . $file, $name . $file);
        }
    }
}
