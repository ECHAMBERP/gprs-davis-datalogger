<?php

/*

  This script reads the text files posted by the stations and appends them 
  to a main "logging" one on the server (here : "uploaded-stations-data.txt")

 * Author : Previmeteo (www.previmeteo.com)
 *
 * Project web site : http://oses.previmeteo.com/
 *
 * License: GNU GPL v2 (see License.txt)
 *
 * Creation date : 2014/02/14
 
*/

 
 
 
$strUploadedFileFormName = "file";

$strUploadedFilePathName = "uploaded-stations-data.txt";


if ($_FILES[$strUploadedFileFormName]['error'] == UPLOAD_ERR_OK 
     && is_uploaded_file($_FILES[$strUploadedFileFormName]['tmp_name'])) { 

  $arrayFileContent = file($_FILES[$strUploadedFileFormName]['tmp_name']);

}

else {

    header("HTTP/1.1 400 BAD REQUEST");
    
    print("You must POST your data !!!");
    
    exit();

}



$success = true;

for($i = 0 ; $i < count($arrayFileContent) ; $i++) {

    try {

        $fp = fopen($strUploadedFilePathName, "a");
    
        fwrite($fp, trim($arrayFileContent[$i])."\n");
        
        fclose($fp);
        
    }
    
    catch(Exception $e) {
    
        print("Error while logging the data !!!\n");
        $success = false;
              
    }
        
}


if($success) print("OK\n");     



?>