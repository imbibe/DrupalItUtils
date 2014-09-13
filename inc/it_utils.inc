<?php

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
