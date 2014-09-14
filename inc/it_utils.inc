<?php
// 
// Imbibe Technologies Pvt Ltd® - http://imbibe.in
// Copyright (c) 2014
// by Imbibe Technologies Pvt Ltd
// 
// This software and associated files including documentation (the "Software") is goverened by Microsoft Public License (MS-PL),
// a copy of which is included with the Software as a text file, License.txt.

function it_utils_disable_output_buffering () {
	while(ob_get_level() > 0) {
		ob_end_clean ();
	}
}

function it_utils_send_attachment ($file_path, $attachment_name = NULL, $contentType = 'application/octet-stream') {
	if($attachment_name == NULL) {
		$attachment_name = basename($file_path);
	}
	
	header("Pragma: public");
	header("Expires: 0");
	header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
	//header('Cache-Control: must-revalidate');
	header("Cache-Control: private", false);
	header("Content-Type: " . $contentType);
	header("Content-Disposition: attachment; filename=" . $attachment_name . ";" );
	header("Content-Transfer-Encoding: binary");
	header("Content-Length: " . filesize($file_path));
	header('Content-Description: File Transfer');

	readfile($file_path);
}
