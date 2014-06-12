<?php

/**
 * Anti SQL Injection attacks
 * If paramater is an array, returns array or is an string, returns single string
 * @param        string $object
 * @uses        mysql_real_escape_string
 * @uses        trim
 * @uses        strip_tags
 * @returns        string
 */
function filter_injection( $object ) 
{
        $object = htmlspecialchars( $object );
        $object = trim( $object );
        $object = strip_tags( $object );
        $object = mysql_real_escape_string( $object );
        return $object;
}
