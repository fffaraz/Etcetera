function checkCodeMeli(code)
{
  
  var L=code.length;
  
  if(L<11 || parseInt(code,10)==0) return false;
  
  if(parseInt(code.substr(3,6),10)==0) return false;
  var c=parseInt(code.substr(10,1),10);
  var d=parseInt(code.substr(9,1),10)+2;
  var z=new Array(29,27,23,19,17);
  var s=0;
  for(var i=0;i<10;i++)
    s+=(d+parseInt(code.substr(i,1),10))*z[i%5];
  s=s%11;if(s==10) s=0;
  return (c==s);

}
