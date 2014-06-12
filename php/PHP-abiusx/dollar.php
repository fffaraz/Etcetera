<?php 
function numbertr($number)
{
	return strtr($number,array(
			"1"=>"۱",
			"2"=>"۲",
			"3"=>"۳",
			"4"=>"۴",
			"5"=>"۵",
			"6"=>"۶",
			"7"=>"۷",
			"8"=>"۸",
			"9"=>"۹",
			"0"=>"۰",
			","=>"<sub>/</sub>",
			
	));
	
}
function curl($URL)
{
	// create curl resource
	$ch = curl_init();

	// set url
	curl_setopt($ch, CURLOPT_URL, $URL);

	//return the transfer as a string
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
// 	curl_setopt($ch, CURLOPT_HEADER, 1);

	// $output contains the output string
	$output = curl_exec($ch);

	// close curl resource to free up system resources
	curl_close($ch);
	return $output;
}
const PROFIT_RATE=300;
function Price_Jalali()
{
		
	$data=curl("http://www.sarafijalali.us");
	
	$start=strpos($data,"نرخ اسکناس در صرافی جلالی‌");
	if ($start===false) return false;
	$rest=substr($data,$start);
	
	preg_match_all("/دلار آمريکا.*?<td>(\d*).*?<td>(\d*)/ms",$rest,$matches);
	if (count($matches)==3)
		return array("Sell"=>$matches[1][0]*10,"Buy"=>$matches[2][0]*10);
	return false;
}
function Price_2gheroon()
{
	$data=curl("http://2zaar.com/1rate/__prices.html");
	
	preg_match_all('/<tr class="odd"><td> <\/td><td class="priceTitle">دلار<\/td><td class="priceValue" id="v3_40a"> <\/td><td id="c3_40a" class=" same">(\d*)<\/td><\/tr>/ms',$data,$matches);
	if (count($matches)==2)
		return array("Sell"=>$matches[1][0]*10,"Buy"=>$matches[1][0]*10-PROFIT_RATE);
	else
		return false;
	
}
const CACHE_FILE="/tmp/usd_price_cache";
function Cache_Store($Prices)
{
	return file_put_contents(CACHE_FILE,serialize($Prices));
}
function Cache_Restore()
{
	if (!file_exists(CACHE_FILE)) return array();
	if (filemtime(CACHE_FILE)<time()-60*60*1) //1 hour old
		unlink(CACHE_FILE);
	if (!file_exists(CACHE_FILE)) return array();
	if ($data=file_get_contents(CACHE_FILE))
		return unserialize($data);
}
$Prices=Cache_Restore();
$sources=array("Jalali"=>"صرافی"
		,"2gheroon"=>"سایت دو قرون");

foreach ($sources as $key=>$source)
	if (!isset($Prices[$key]))
		$Prices[$key]=call_user_func("Price_{$key}");
	
Cache_Store($Prices);
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html dir="rtl">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<META name="keywords" content="cert">
<META name="description" content="">
<link rel="shortcut icon" href="favicon.ico" />
<title>sarra.fi نرخ روز دلار</title>
<style>
a {
text-decoration: none;
}
#main {
	text-align:center;
	background-color:white;
	opacity:.9;
	padding:10px;
	width:400px;
	margin:auto;
	border:2px outset gray;
}
#main table {
	margin:auto;
}
</style>
<base href='http://sarra.fi/' />
</head>
<body style="background-image:url(img/bg.jpg); background-repeat:repeat-x;">

<div align="center">

<table width="930" height="500" border="0" cellpadding="0" cellspacing="0" style="background-image:url(img/globe.png); margin-top: 30px;">
<tr height="90"></tr>
<tr>
<td>
<div id='main'>
<h2>قیمت دلار</h2>
<table cellspacing='0' cellpadding='10'>
<thead>
<tr>
<th></th>
<th>خرید</th>
<th>فروش</th>
</thead>
<tbody>
<?php 
$sumBuy=$sumSell=0;
foreach ($Prices as $key=>$price)
{
	echo "<tr>";
	if ($price===false)
	{
		echo "<td colspan='3'>یافت نشد</td></tr>";
		continue;
	}
	echo "<td>".$sources[$key].":</td>";
	
	$buy=$price['Buy'];
	$sumBuy+=$buy;
	echo "<td><strong>".numbertr(number_format($buy))."</strong> ریال</td>";
	$sell=$price['Sell'];
	$sumSell+=$sell;
	echo "<td><strong>".numbertr(number_format($sell))."</strong> ریال</td>";
	
	echo "</tr>";
	
}	
?>
<tr><td colspan='3'><hr/></td></tr>
<tr style='font-weight:bolder;'>
<td>میانگین</td>
<td><?php echo numbertr(number_format($sumBuy/count($Prices)));?> ریال</td>
<td><?php echo numbertr(number_format($sumSell/count($Prices)));?> ریال</td>
</tr>
</tbody>
</table>
</div>
</td>
</tr>
<tr></tr>
</table>

</div>
<p align="center">Designed and Hosted by <a href="https://etebaran.com">Etebaran Informatics</a></p>
<div style='text-align:center;font-size:8px;'>منبع دیگری می‌شناسید؟
<a href='mailto:sarrafi@etebaran.com'>
به ما معرفی کنید
</a> 
</div>
</body>
</html>

