
<?php
/**
 * 
 * Iran.tc SMS center management module
 * @depends on SoapClient or nusoap  
 * @author abiusx
 * @version 1.1 stable
 * 
 */
class SmsCenter
{
	public $Username;
	public $Password;
	public $Number;
	
	protected $WSDL_URL = "http://webservice.iran.tc/?wsdl";
	/**
	 * Returns the web service interface to be used for use by other functions
	 * @return SoapClient 
	 */
	protected function Service()
	{
		if ($this->_service === null)
		{
			if (class_exists ( "SoapClient" ))
				$this->_service = new SoapClient ( $this->WSDL_URL );
			else 
				if (class_exists ( "soapclient" ))
					$this->_service = new soapclient ( $this->WSDL_URL );
				else
					throw new Exception ( "No soap client available." );
		}
		return $this->_service;
	}
	
	private $_service = null;
	
	function __construct($Username, $Password, $Number)
	{
		$this->Username = $Username;
		$this->Password = $Password;
		$this->Number = $Number;
	}
	
	function Send($To, $Content)
	{
		if (substr ( $To, 0, 3 ) == '+98')
			$To = '0' . substr ( $To, 3 );
		$ID = $this->Service ()->SendSMS ( $this->Username, $this->Password, $To, $Content, $this->Number );
		return $ID;
	}
	function GetBalance()
	{
		return $this->Service ()->CREDIT_LINESMS ( $this->Username, $this->Password, $this->Number );
	}
	function GetSentCount()
	{
		return $this->Service ()->COUNT_SENDSMS ( $this->Username, $this->Password );
	}
	function GetUnreadCount()
	{
		return $this->Service ()->COUNT_GETSMS ( $this->Username, $this->Password, 1 );
	}
	/**
	 * 
	 * This is supposed to return number of messages in inbox but is not reliable
	 */
	function GetInboxCount()
	{
		return $this->Service ()->COUNT_GETSMS ( $this->Username, $this->Password, 2 );
	}
	/**
	 * 
	 * 
	 * @param integer $SMSID
	 * @return SmsStatus
	 */
	function GetStatus($SMSID)
	{
		$Status = $this->Service ()->StatusSMS ( $this->Username, $this->Password, $SMSID );
		if ($Status)
			return new SmsStatus ( $Status );
		else
			return null;
	}
	/**
	 * 
	 * Retrieves an sms if existing and returns it. Otherwise nul
	 * @return SmsInstance
	 */
	function Receive()
	{
		
		$r = $this->Service ()->SMS_GET ( $this->Username, $this->Password, 1 ); //get unread count
		if ($r < 1)
			return null;
		$Params = $this->Service ()->SMS_GET ( $this->Username, $this->Password, 2 ); //get body unparsed
		// samples: #5915309~+989123874634~9109011716~Testing Something
		$a = explode ( "~", $Params );
		return new SmsInstance ( $a [1], null, $a [3], $a [0] );
	}
	/**
	 *
	 * This function runs Receive untill no more messages are left. Returns them in an array.
	 * Would probably take too long, keep in mind.
	 */
	function ReceiveAll()
	{
		$out = array ();
		while ( $r = $this->Receive () )
			$out [] = $r;
		if (count ( $out ))
			return $out;
		else
			return null;
	}


}
/**
 * 
 * This class represents a SMS. 
 * Currently used to return read sms
 * @author abiusx
 * @version 1.0
 */
class SmsInstance
{
	public $From;
	public $To;
	public $Body;
	public $ID;
	public $Timestamp;
	function __construct($From = null, $To = null, $Body = null, $ID = null, $Timestamp = null)
	{
		$this->From = $From;
		$this->To = $To;
		$this->Body = $Body;
		$this->ID = $ID;
		$this->Timestamp = $Timestamp ?  : time ();
	}
}
/**
 * 
 * This class represents status of a sent sms.
 * @author abiusx
 * @version 1.0
 */
class SmsStatus
{
	function __toString()
	{
		return $this->statusCode . "";
	}
	protected $statusCode;
	function __construct($StatusCode)
	{
		$this->statusCode = $StatusCode;
	}
	function IsDelivered()
	{
		return $this->statusCode | SmsStatus::$Delivered;
	}
	function IsInQueue()
	{
		return $this->statusCode | SmsStatus::$InQueue;
	}
	function ReachedTC()
	{
		return $this->statusCode | SmsStatus::$ReachedTC;
	}
	function Failed()
	{
		return $this->statusCode | SmsStatus::$Not;
	}
	static $Delivered = 1;
	static $Not = 2;
	static $InQueue = 4;
	static $ReachedTC = 8;
	static $NotTC = 16;
}

/**
 * 
 * Extended SMS Center class
 * relies on a database to handle sent and receive messages
 * @author abiusx
 * @version 1.1
 */
class SmsCenterExtended extends SmsCenter
{
	public $TablePrefix = "sms_";
	
	function SetupDB()
	{
		$this->sql ( "CREATE TABLE IF NOT EXISTS `{$this->TablePrefix}receive` (
			  `ID` int(11) NOT NULL AUTO_INCREMENT,
			  `From` varchar(32) COLLATE utf8_bin NOT NULL,
			  `To` varchar(32) COLLATE utf8_bin NOT NULL,
			  `SMSID` varchar(32) COLLATE utf8_bin NOT NULL,
			  `Body` varchar(2048) COLLATE utf8_bin NOT NULL,
			  `Timestamp` int(11) NOT NULL,
			  PRIMARY KEY (`ID`),
			  UNIQUE KEY `SMSID` (`SMSID`),
			  KEY `From` (`From`),
			  KEY `Timestamp` (`Timestamp`)
				) " );
		$this->sql ( "CREATE TABLE IF NOT EXISTS `{$this->TablePrefix}send` (
			  `ID` int(11) NOT NULL AUTO_INCREMENT,
			  `SMSID` varchar(32) COLLATE utf8_bin NOT NULL,
			  `Status` int(11) NOT NULL,
			  `Timestamp` int(11) NOT NULL,
			  `To` varchar(32) COLLATE utf8_bin NOT NULL,
			  `Body` varchar(2048) COLLATE utf8_bin NOT NULL,
			  `From` varchar(32) COLLATE utf8_bin NOT NULL,
			  PRIMARY KEY (`ID`),
			  UNIQUE KEY `SMSID` (`SMSID`),
			  KEY `Timestamp` (`Timestamp`),
			  KEY `To` (`To`)
			) " );
	}
	
	protected function sql($Query)
	{
		$args = func_get_args ();
		return call_user_func_array ( $this->sqlFunction, $args );
	}
	protected $sqlFunction = null;
	/**
	 * 
	 * Enter description here ...
	 * @param String $Username
	 * @param String $Password
	 * @param String $Number
	 * @param callback $sqlFunction, this should be a function which accepts parameters for prepared sql running,
	 * 	returns 2D array on SELECT, affected rows on UPDATE and DELETE and insert ID on INSERT.
	 */
	function __construct($Username, $Password, $Number, $sqlFunction)
	{
		$this->Username = $Username;
		$this->Password = $Password;
		$this->Number = $Number;
		$this->sqlFunction = $sqlFunction;
		if (! is_callable ( $this->sqlFunction ))
			throw new Exception ( "Invalid SQL callback" );

		$this->SetupDB();
	}
	function GetStatus($SMSID)
	{
		$Status = parent::GetStatus ( $SMSID );
		if ($Status)
		{
			
			$res = $this->sql ( "SELECT * FROM {$this->TablePrefix}send WHERE SMSID=?", $SMSID );
			if (count ( $res )) //update status in DB
				$r = $this->sql ( "UPDATE {$this->TablePrefix}send SET Status=? WHERE ID=?", $Status, $res [0] [ID] );
			return $Status;
		}
		else
			return null;
	
	}
	function Send($To, $Content)
	{
		$r = parent::Send ( $To, $Content );
		if ($r > 10)
		{
			$IID = $this->sql ( "INSERT INTO {$this->TablePrefix}send (SMSID,Status,Timestamp,`To`,Body,`From`) VALUES
				(?,?,?,?,?,?)", $r, parent::GetStatus ( $r ), time (), $To, $Content, $this->Number );
			return $r;
		}
		else
			return $r;
	}
	function Receive()
	{
		$sms = parent::Receive ();
		$IID = $this->sql ( "INSERT INTO {$this->TablePrefix}receive (`From`,`To`,SMSID,Body,Timestamp) 
				VALUES (?,?,?,?,?)", $sms->From, $this->Number, $sms->ID, $sms->Body, $sms->Timestamp );
		return $sms;
	
	}

}

require_once 'sql.php';

$sce = new SmsCenterExtended ( "etebaran", "AIB16BR6p3LyXGB", "30009900662013", 'SQL' );
var_dump(
$sce->Send("09379704524", "salam vahid. baba behem zang zad goft gharare beri masjed. montazeram zang bezani. mesle diruzam zood ghat' nakon. ghashang 3 4 bar tulani gushi ro negah dar. boos")
);
//echo $sce->GetStatus ( "6059453020" );
//$sce->GetStatus(6060351050);
var_dump($sce->ReceiveAll());
