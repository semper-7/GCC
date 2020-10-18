<html>
</head>
<title>MyHome</title>
<?
$lines = file('./Log/'.date('Ymd').'.log');
$line = $lines[count($lines)-1];
$s = explode(" ", $line);
$date = date('Y.m.d').' '.$s[0];
$p = ['black','red','yellow','orange'];
$cfg = file('log.cfg');
 for ($i = 0; $i < 8; $i++) {
  if (s[$i+1] < 64) {
  $c[$i] = 'rgb('.($s[$i+1] << 2).',32,255)';
  } else {
  $c[$i] = 'rgb(255,32,'.(255 - (($s[$i+1] - 64) << 2 )).')';
  }
 $s[$i] = substr_replace($s[$i+1],'',-1,1);
 }
if (substr($s[9],0,1) == '0') $s[8] = substr($s[9],1,4);
$f = '#888';
if (((0 + $s[10]) & 4) > 3) $f = '#f60';
?>
<meta HTTP-EQUIV="Refresh" CONTENT="15">
<style type="text/css">
.border { 
 position:absolute;
 border: solid 2px black; }
.TALayer {
 position:absolute;
 text-align: center;
 font-size:24px;
 line-height:36px;
 color:yellow;
 left: 42px; width:100px; height:36px; }
.CO_Top {
 position:absolute;
 background: <?echo $c[0]?>; }
.Tg_Bot {
 position:absolute;
 background: <?echo $c[5]?>; }
.TA_Bot {
 position:absolute;
 background: <?echo $c[4]?>; }
.CO { 
 position:absolute;
 top:260px;width:8px;height:40px;
 background: <?echo $c[0]?>;
 border: solid 2px black; }
.Pump {
 position:absolute;
 width: 0; height: 0;
 border: 10px solid ;
 border-top-color: #072;
 border-bottom-color: #072;
 border-right-color: #072;
 border-left-color: <?echo $p[$s[10]&3]?>; }
</style> 
</head>
<body style="color:red;font-weight:bold;font-family:sans-serif;font-size:16px;">
<div class="border" style="background:#CEC;left:0px;top:0px;width:490px;height:390px;text-align:center;font-size:24px;">Boiler room</div>
<div style="position:absolute;background:#EEC;left:2px;top:240px;width:180px;height:152px;"></div>
<div style="position:absolute;left:180px;top:60px;"><?echo $date?></div>
<a href="log.cfg?s=<?echo date('dHis')?>"style="position:absolute;left:180px;top:90px;color:maroon;">View configurations</a>
<a href="logview.php?log=<?echo date('Ymd')?>"style="position:absolute;left:180px;top:120px;color:maroon;">View log</a>
<a href="files.php"style="position:absolute;left:180px;top:150px;color:maroon;">Download files</a>
<div style="position:absolute;left:2px;top:350px;width:180px;height:40px;line-height:40px;font-size:24px;text-align:center;">Home: <?echo $s[6]?>&#8451</div>
<div style="position:absolute;background:#CEE;left:182px;top:350px;width:308px;height:40px;line-height:40px;font-size:24px;text-align:center;">Vegetable store: <?echo $s[8]?>&#8451</div>
<div style="position:absolute;left:350px;top:293px;"><?echo $s[5]?>&#8451</div>
<div style="position:absolute;left:370px;top:150px;"><?echo $s[7]?>&#8451</div>

<div class="border" style="left:40px;top:40px;width:100px;height:180px;"></div>
<div class="TALayer" style="top:42px;background:<?echo $c[0]?>;"><?echo $s[0]?>&#8451</div>
<div class="TALayer" style="top:78px;background:<?echo $c[1]?>;"><?echo $s[1]?>&#8451</div>
<div class="TALayer" style="top:114px;background:<?echo $c[2]?>;"><?echo $s[2]?>&#8451</div>
<div class="TALayer" style="top:150px;background:<?echo $c[3]?>;"><?echo $s[3]?>&#8451</div>
<div class="TALayer" style="top:186px;background:<?echo $c[4]?>;"><?echo $s[4]?>&#8451</div>

<div class="border" style="left:87px;top:10px;width:6px;height:28px;"></div>
<div class="CO_Top" style="left:89px;top:12px;width:6px;height:30px;"></div>
<div class="border" style="left:10px;top:10px;width:75px;height:6px;"></div>
<div class="CO_Top" style="left:12px;top:12px;width:77px;height:6px;"></div>
<div class="border" style="left:10px;top:18px;width:6px;height:300px;"></div>
<div class="CO_Top" style="left:12px;top:18px;width:6px;height:302px;"></div>
<div class="border" style="left:18px;top:312px;width:380px;height:6px;"></div>
<div class="CO_Top" style="left:18px;top:314px;width:73px;height:6px;"></div>
<div class="CO_Top" style="left:91px;top:314px;width:207px;height:6px;"></div>
<div class="TG_Bot" style="left:298px;top:314px;width:102px;height:6px;"></div>
<div class="border" style="left:36px;top:294px;width:12px;height:2px;"></div>
<div class="CO_Top" style="left:38px;top:296px;width:12px;height:2px;"></div>
<div class="border" style="left:36px;top:298px;width:2px;height:12px;"></div>
<div class="CO_Top" style="left:38px;top:298px;width:2px;height:16px;"></div>
<div class="border" style="left:130px;top:294px;width:12px;height:2px;"></div>
<div class="CO_Top" style="left:132px;top:296px;width:12px;height:2px;"></div>
<div class="border" style="left:140px;top:298px;width:2px;height:12px;"></div>
<div class="CO_Top" style="left:142px;top:298px;width:2px;height:16px;"></div>
<div class="border" style="left:142px;top:210px;width:200px;height:6px;"></div>
<div class="TA_Bot" style="left:142px;top:212px;width:202px;height:6px;"></div>
<div class="border" style="left:336px;top:218px;width:6px;height:92px;"></div>
<div class="TA_Bot" style="left:338px;top:218px;width:6px;height:96px;"></div>
<div class="border" style="left:142px;top:42px;width:324px;height:6px;"></div>
<div class="CO_Top" style="left:142px;top:44px;width:326px;height:6px;"></div>
<div class="border" style="left:460px;top:50px;width:6px;height:150px;"></div>
<div class="CO_Top" style="left:462px;top:50px;width:6px;height:150px;"></div>

<div class="border" style="left:400px;top:200px;width:80px;height:120px;background:#03f;"></div>
<div class="border" style="left:404px;top:200px;width:72px;height:22px;"></div>
<div class="border" style="left:404px;top:224px;width:72px;height:54px;"></div>
<div class="border" style="left:432px;top:210px;width:14px;height:4px;background:#007;"></div>
<div class="border" style="left:410px;top:206px;width:10px;height:10px;background:#FCC;border-radius: 50%"></div>
<div class="border" style="left:404px;top:280px;width:72px;height:45px;background:#444;"></div>
<div class="border" style="left:422px;top:290px;width:36px;height:24px;"></div>
<div class="border" style="left:437px;top:296px;width:6px;height:12px;background:<?echo $f?>;"></div>
<div class="border" style="left:408px;top:300px;width:6px;height:6px;border-radius:50%;"></div>
<div class="border" style="left:426px;top:294px;width:4px;height:4px;border-radius:50%;"></div>
<div class="border" style="left:430px;top:128px;width:20px;height:70px;background:#888;"></div>

<div class="CO" style="left:50px;"></div>
<div class="CO" style="left:60px;"></div>
<div class="CO" style="left:70px;"></div>
<div class="CO" style="left:80px;"></div>
<div class="CO" style="left:90px;"></div>
<div class="CO" style="left:100px;"></div>
<div class="CO" style="left:110px;"></div>
<div class="CO" style="left:120px;"></div>

<div class="border" style="left:270px;top:295px;width:40px;height:40px;border-radius:50%;background:#072;"></div>
<div class="Pump" style="left:289px;top:307px;"></div>

</body>
</html>
