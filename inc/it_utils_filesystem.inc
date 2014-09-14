<?php
// 
// Imbibe Technologies Pvt Ltd® - http://imbibe.in
// Copyright (c) 2014
// by Imbibe Technologies Pvt Ltd
// 
// This software and associated files including documentation (the "Software") is goverened by Microsoft Public License (MS-PL),
// a copy of which is included with the Software as a text file, License.txt.

//http://stackoverflow.com/questions/17708562/zip-all-files-in-directory-and-download-zip-generated
class ItUtils_FlxZipArchive extends ZipArchive {
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
function it_utils_filesystem_compress_file_gz($source, $level = 9){ 
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

function it_utils_filesystem_zip_directory($dirPath, $outputFilePath = NULL) {
	if($outputFilePath == NULL) {
		$outputFilePath = tempnam(sys_get_temp_dir(), 'tmp');
	}

	$za = new ItUtils_FlxZipArchive;
	$res = $za->open($outputFilePath, ZipArchive::CREATE);
	if($res === TRUE) {
		$za->addDir($dirPath, basename($dirPath));
		$za->close();
	} else{
		return (NULL);
	}
	
	return ($outputFilePath);
}

function it_utils_get_public_files_directory() {
	$dir = variable_get('file_public_path', conf_path() . '/files');
	return ($dir);
}
