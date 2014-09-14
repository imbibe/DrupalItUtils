<?php
// 
// Imbibe Technologies Pvt Ltd® - http://imbibe.in
// Copyright (c) 2014
// by Imbibe Technologies Pvt Ltd
// 
// This software and associated files including documentation (the "Software") is goverened by Microsoft Public License (MS-PL),
// a copy of which is included with the Software as a text file, License.txt.

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

	$form['download_full_filesystem_backup'] = array(
		'#type' => 'button',
		'#value' => t('Download Full File-system backup'),
		'#submit' => array('it_utils_admin_structure_form_download_full_filesystem_backup'),
		'#executes_submit_callback' => TRUE,
	);
	return ($form);
}

//Database backup
function it_utils_admin_structure_form_download_db_backup ($form, &$form_state) {
	module_load_include('inc', 'it_utils', 'inc/it_utils');
	module_load_include('inc', 'it_utils', 'inc/it_utils_db');
	module_load_include('inc', 'it_utils', 'inc/it_utils_filesystem');
	
	_it_utils_admin_init_long_running_request();
	it_utils_disable_output_buffering();

	global $databases;
	
	$dbhost = $databases['default']['default']['host'];
	$dbname = $databases['default']['default']['database'];
	$dbuser = $databases['default']['default']['username'];
	$dbpass = $databases['default']['default']['password'];

	/*$temp_file = tempnam(sys_get_temp_dir(), 'tmp');
	$command = "mysqldump -h $dbhost -u $dbuser -p$dbpass $dbname > $temp_file";
	system($command);*/
	$temp_file = it_utils_db_backup_tables($dbhost, $dbname, $dbuser, $dbpass);

	$compressed_file = it_utils_filesystem_compress_file_gz($temp_file);
	unlink($temp_file);
	
	$attachment_name = $dbname . date("-Y-m-d-H-i-s") . '.sql.gz';
	it_utils_send_attachment($compressed_file, $attachment_name, 'application/x-gzip');

	unlink($compressed_file);
	exit;
}

//Public Files backup
function it_utils_admin_structure_form_download_public_files_backup ($form, &$form_state) {
	module_load_include('inc', 'it_utils', 'inc/it_utils');
	module_load_include('inc', 'it_utils', 'inc/it_utils_filesystem');

	_it_utils_admin_init_long_running_request();
	it_utils_disable_output_buffering();
	
	$dir = it_utils_get_public_files_directory();	
	$compressed_file = it_utils_filesystem_zip_directory($dir);

	it_utils_send_attachment($compressed_file, basename($dir) . '.zip', 'application/zip');

	unlink($compressed_file);
	exit;
}

//Full File-system backup
function it_utils_admin_structure_form_download_full_filesystem_backup ($form, &$form_state) {
	module_load_include('inc', 'it_utils', 'inc/it_utils');
	module_load_include('inc', 'it_utils', 'inc/it_utils_filesystem');

	_it_utils_admin_init_long_running_request();
	it_utils_disable_output_buffering();
	
	$dir = DRUPAL_ROOT;
	$compressed_file = it_utils_filesystem_zip_directory($dir);

	it_utils_send_attachment($compressed_file, basename($dir) . '.zip', 'application/zip');

	unlink($compressed_file);
	exit;
}

function _it_utils_admin_init_long_running_request () {
	//10 minutes
	set_time_limit(600);
}
