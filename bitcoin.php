<?php
$btc = decode_btc('1Eym7pyJcaambv8FG4ZoU8A4xsiL9us2zz');
var_dump($btc);
var_dump(encode_btc($btc));
$btc = decode_btc('1111111111111111111114oLvT2');
var_dump($btc);
var_dump(encode_btc($btc));

function hash_sha256($string) {
  if (function_exists('hash')) return hash('sha256', $string, true);
  if (function_exists('mhash')) return mhash(MHASH_SHA256, $string);
  // insert native php implementation of sha256 here
  throw new Exception('Too lazy to fallback when the guy who configured php was lazy too');
}

function encode_btc($btc) {
  $btc = chr($btc['version']).pack('H*', $btc['hash']);
  if (strlen($btc) != 21) return false;
  $cksum = substr(hash_sha256(hash_sha256($btc)), 0, 4);
  return base58_encode($btc.$cksum);
}

function decode_btc($btc) {
  $btc = base58_decode($btc);
  if (strlen($btc) != 25) return false; // invalid
  $version = ord($btc[0]);
  $cksum = substr($btc, -4);
  // checksum is double sha256 (take 4 first bytes of result)
  $good_cksum = substr(hash_sha256(hash_sha256(substr($btc, 0, -4))), 0, 4);
  if ($cksum != $good_cksum) return false;
  return array('version' => $version, 'hash' => bin2hex(substr($btc, 1, 20)));
}

function base58_encode($string) {
  $table = '123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz';

  $long_value = gmp_init(bin2hex($string), 16);

  $result = '';
  while(gmp_cmp($long_value, 58) > 0) {
    list($long_value, $mod) = gmp_div_qr($long_value, 58);
    $result .= $table[gmp_intval($mod)];
  }
  $result .= $table[gmp_intval($long_value)];

  for($nPad = 0; $string[$nPad] == "\0"; ++$nPad);

  return str_repeat($table[0], $nPad).strrev($result);
}

function base58_decode($string) {
  $table = '123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz';
  static $table_rev = null;
  if (is_null($table_rev)) {
    $table_rev = array();
    for($i=0;$i<58;++$i) $table_rev[$table[$i]]=$i;
  }

  $l = strlen($string);
  $long_value = gmp_init('0');
  for($i=0;$i<$l;++$i) {
    $c=$string[$l-$i-1];
    $long_value = gmp_add($long_value, gmp_mul($table_rev[$c], gmp_pow(58, $i)));
  }

  // php is lacking binary output for gmp
  $res = pack('H*', gmp_strval($long_value, 16));

  for($nPad = 0; $string[$nPad] == $table[0]; ++$nPad);
  return str_repeat("\0", $nPad).$res;
}
