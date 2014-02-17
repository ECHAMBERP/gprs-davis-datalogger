<?

/*

 * This script returns the GMT date and time to the HTTP client. 
 *
 * This data can be used to (re)sync the embedded RTC in the GPRS stations.
 *
 * Author : Previmeteo (www.previmeteo.com)
 *
 * Project web site : http://oses.previmeteo.com/
 *
 * License: GNU GPL v2 (see License.txt)
 *
 * Creation date : 2014/02/14
 
*/




$timestampReceivedAt = mktime();


$dateTimeNowUTC = gmdate("Y-m-d H:i:s");

print("GMT : $dateTimeNowUTC");

?>