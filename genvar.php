<?php

require("libs/common.php");
define('LORENZ',BASEDIR.'/lorenz');
$climbs = climb_list();

function fuzzy_strmatch($a,$b){
  $a = preg_replace('/[^a-z]/','-',strtolower(trim($a)));
  $b = preg_replace('/[^a-z]/','-',strtolower(trim($b)));
  return (strcmp($a,$b) == 0);
}

// O(n) but each iter wants 3 pow() and 1 sqrt(). Might be slow for large n.
function nearest_neighbor($symbols,$x,$nna,$mask,$exclude=-1){
  if($nna == "D") return dabby_nearest_neighbor($symbols,$x[0],$exclude);
 
  $found = -1;
  $distmin = -1;
  foreach($symbols as $i => $d){
    $dist = norm($symbols[$i]["x"],$x,$mask);
    if(($found < 0) || ($dist < $distmin)){
      $found = $i;
      $distmin = $dist;
    }
  }
  return $found;
}

// only in x-dimension. fast but bad.
function dabby_nearest_neighbor($symbols,$x,$exclude=-1){
  $found = -1;
  foreach($symbols as $i => $d){
    if($symbols[$i]["x"][0] < $x) continue;
    if(($exclude >= 0) && ($exclude == $i)) continue;
    if(($found < 0) || ($symbols[$i]["x"][0] < $symbols[$found]["x"][0])) $found = $i;
  }
  return $i;
}

// might consider doing infinity norm if 2-norm is too slow
function norm($x1,$x2,$mask){
  $r = 0;
  foreach($x1 as $i => $x1e){
    $r += $mask[$i]*pow($x2[$i]-$x1e,2);
  }
  return sqrt($r);
}

function two_dim_project($d,$mask){
  if(is_array($mask)) $mask = implode("",$mask);
  if(($mask == "001") || ($mask == "010") || ($mask == "011")){
    $x = $d[1];
    $y = $d[2];
  }else if(($mask == "100") || ($mask == "110") || ($mask == "111")){
    $x = $d[0];
    $y = $d[1];
  }else{
    $x = $d[0];
    $y = $d[1];
  }
  return array($x,$y);
}

function make_svg($rid,$input,$var,$mask){
  $margin = 10;
  $height = 600+$margin;
  $width = 800+$margin;
  $fh = fopen("figs/$rid.svg","w");
  $preamble = <<<EOF
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:dc="http://purl.org/dc/elements/1.1/"
   xmlns:cc="http://creativecommons.org/ns#"
   xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:xlink="http://www.w3.org/1999/xlink"
   width="$width"
   height="$height"
   id="svg2"
version="1.0">
EOF;
  fwrite($fh,$preamble);
  $min = array(NULL,NULL);
  $max = array(NULL,NULL);
  foreach($input as $d){
    list($x,$y) = two_dim_project($d["x"],$mask);
    if(($min[0] == NULL) or ($min[0] > $x)) $min[0] = $x;
    if(($min[1] == NULL) or ($min[1] > $y)) $min[1] = $y;
    if(($max[0] == NULL) or ($max[0] < $x)) $max[0] = $x;
    if(($max[1] == NULL) or ($max[1] < $y)) $max[1] = $y;
  }
  foreach($var as $d){
    list($x,$y) = two_dim_project($d["x"],$mask);
    if(($min[0] == NULL) or ($min[0] > $x)) $min[0] = $x;
    if(($min[1] == NULL) or ($min[1] > $y)) $min[1] = $y;
    if(($max[0] == NULL) or ($max[0] < $x)) $max[0] = $x;
    if(($max[1] == NULL) or ($max[1] < $y)) $max[1] = $y;
  }
  $xr = $max[0]-$min[0];
  $yr = $max[1]-$min[1];
  $last = array(NULL,NULL);
  foreach($input as $d){
    list($x,$y) = two_dim_project($d["x"],$mask);
    $sx = ($x-$min[0])*(($width-$margin)/$xr) + ($margin/2);
    $sy = ($y-$min[1])*(($height-$margin)/$yr) + ($margin/2);
    if($last[0] != NULL){
      fwrite($fh,'<line x1="'.$last[0].'" y1="'.$last[1].'" x2="'.$sx.'" y2="'.$sy.'" style="stroke:rgb(0,0,0);stroke-width:2" />'."\n");
    }
    $last[0] = $sx;
    $last[1] = $sy;
  }
  $last = array(NULL,NULL);
  foreach($var as $d){
    list($x,$y) = two_dim_project($d["x"],$mask);
    $sx = ($x-$min[0])*(($width-$margin)/$xr) + ($margin/2);
    $sy = ($y-$min[1])*(($height-$margin)/$yr) + ($margin/2);
    list($xi,$yi) = two_dim_project($input[$d["vi"]]["x"],$mask);
    $sxi = ($xi-$min[0])*(($width-$margin)/$xr) + ($margin/2);
    $syi = ($yi-$min[1])*(($height-$margin)/$yr) + ($margin/2);

    if($last[0] != NULL){
      fwrite($fh,'<line x1="'.$last[0].'" y1="'.$last[1].'" x2="'.$sx.'" y2="'.$sy.'" style="stroke:rgb(0,0,255);stroke-width:2" />'."\n");
    }
    fwrite($fh,'<line x1="'.$sxi.'" y1="'.$syi.'" x2="'.$sx.'" y2="'.$sy.'" style="stroke:rgb(168,168,200);stroke-width:1" />'."\n");
    $last[0] = $sx;
    $last[1] = $sy;
  }
  fwrite($fh,"</svg>");
  fclose($fh);

  $cmd = "convert -resize 200x figs/$rid.svg figs/$rid.png";
  $output = shell_exec($cmd);
}

function make_variation(){
  global $current_user; 

  $nna = $_REQUEST['nna'];
  $nndm = $_REQUEST['nndm'];
  $pt = $_REQUEST['pt'];
  $rm = $_REQUEST['rm'];
  $skip = $_REQUEST['skip'];
  $ric = $_REQUEST['refic'];
  $vic = $_REQUEST['varic'];
  $climbs = $_REQUEST['climbs'];

  // sanity checks
  if(!in_array($nna,array("B","D"))) return -1;
  if(!is_numeric($pt)) return -1;
  if(!is_numeric($rm)) return -1;
  if(!is_numeric($skip)) return -1;
  foreach($ric as $i) if(!is_numeric($i)) return -1;
  foreach($vic as $i) if(!is_numeric($i)) return -1;
  if(count($climbs) <= 0) return -1;
  foreach($climbs as $i) if(!is_numeric($i)) return -1;

  // get input data
  $c2 = implode(",",$climbs);
  $input = array();
  $sql = "SELECT route_id,seq_no,hand,move FROM moves WHERE route_id IN ($c2) ORDER BY route_id,seq_no"; 
  $r = $current_user->db->query($sql);
  if(!$r) return -2;
  while($row = pg_fetch_assoc($r)) array_push($input,$row); 
  $var_len = count($input);

  // fudge matches and add a couple boolean cols we'll need
  foreach($input as $i => $d){
    if(preg_match('/^\s*match\s*$/i',$d["move"])){
      if($i >= 1) $input[$i]["move"] = $input[$i-1]["move"]." (match)";
    }
    $input[$i]["error"] = FALSE;
    $input[$i]["nnn"] = FALSE;
  }

  // begin a transaction
  $r = $current_user->db->query("BEGIN");
  if(!$r) return -3;

  // insert variation route

  $name = "Variation on ".implode(",",$climbs);
  $sql = "INSERT INTO routes (user_id,name,is_variation,nna,".
         "dim_mask,max_repeat,prec_trunc,transient,".
         "rx,ry,rz,vx,vy,vz) VALUES ($1,$2,'t',$3,".
         "$4,$rm,$pt,$skip,".
         "$5,$6,$7,$8,$9,$10)";
  $vals = array($current_user->user_id,$name,$nna,
                implode("",$nndm),$ric[0],$ric[1],$ric[2],$vic[0],$vic[1],$vic[2]);
  $r = $current_user->db->query_params($sql,$vals);
  if(!$r) return -3;
  $sql = "SELECT currval('routes_route_id_seq')";
  $r = $current_user->db->query($sql);
  if(!$r) return -3;
  list($rid) = pg_fetch_array($r);
  foreach($climbs as $ridin){
    $sql = "INSERT INTO inputs (in_id,out_id) VALUES ($1,$2)";
    $r = $current_user->db->query_params($sql,array($ridin,$rid));
    if(!$r) return -3;
  }

  // get trajectories
  $ric2 = implode(" ",$ric);
  $cmd = LORENZ." $var_len $skip $ric2";
  $output = shell_exec($cmd);
  // first, append reference trajectory data to input symbol data
  foreach(explode("\n",trim($output)) as $i => $line){
    list($t,$x,$y,$z) = explode(" ",trim($line));
    if($pt > 0){
      $x = truncate_precision($x,$pt);
      $y = truncate_precision($y,$pt);
      $z = truncate_precision($z,$pt);
    }
    $input[$i]["x"] = array($x,$y,$z);
  }
  $var = array();
  $vic2 = implode(" ",$vic);
  $cmd = LORENZ." $var_len $skip $vic2";
  $output = shell_exec($cmd);
  $last = -1;
  $repeat_count = 0;
  $vs = 0;
  // then shift things around using nearest neighbor
  foreach(explode("\n",trim($output)) as $i => $line){
    list($t,$x,$y,$z) = explode(" ",trim($line));
    if($pt > 0){
      $x = truncate_precision($x,$pt);
      $y = truncate_precision($y,$pt);
      $z = truncate_precision($z,$pt);
    }
    $vi = nearest_neighbor($input,array($x,$y,$z),$nna,$nndm);
    if($vi < 0){ # shouldn't be possible...
      $vi = $i;
      $input[$i]["error"] = TRUE;
    }
    if($last >= 0){
      // naively check for repeats either in index number or in terms of actual move.
      if(($last == $vi) || fuzzy_strmatch($input[$last]["move"],$input[$vi]["move"])) $repeat_count += 1;
      else $repeat_count = 0;

      if(($rm >= 0) && ($repeat_count > $rm)){
        // find next nearest neighbor
        $vi = nearest_neighbor($input,array($x,$y,$z),$nna,$nndm,$vi);  
        if($vi < 0){ # shouldn't be possible
          $vi = $i;
          $input[$i]["error"] = TRUE;
        }
        $repeat_count = 0;
        $input[$vi]["nnn"] = TRUE;
      }
    }
    array_push($var,array("x" => array($x,$y,$z),"vi" => $vi));

    $sql = "INSERT INTO moves (seq_no,route_id,hand,move,delta,nnn,error) VALUES ($1,$2,$3,$4,$5,$6,$7)";
    $r = $current_user->db->query_params($sql,array($vs,$rid,$input[$vi]["hand"],$input[$vi]["move"],abs($vi-$i),(($input[$vi]["nnn"]) ? 't' : 'f'),(($input[$vi]["error"]) ? 't' : 'f')));
    if(!$r) return -3;

    $last = $vi;
    $vs += 1;
  }

  make_svg($rid,$input,$var,$nndm);

  $r = $current_user->db->query("COMMIT");
  if(!$r) return -3; 

  return $rid;
}

if(isset($_REQUEST['nna'])){
  $ret = make_variation();
  if($ret == -1) wail("Naughty looking inputs. Aborted.");
  else if($ret == -2) wail("Failed to fetch route data.");
  else if($ret == -3) wail("Failed to insert variation route data.");
  else{
    list($header,$moves) = do_lookup_climb($ret);
    if(!$header) wail("Failed to lookup climb I just generated, wtf.");
  }
}

$smarty = make_smarty();
if($header){
  $smarty->assign("css","popup");
  $smarty->assign("simple",1);
  $smarty->display("top.tpl");
  $smarty->assign("header",$header);
  $smarty->assign("moves",$moves);
  $smarty->assign("mine",TRUE);
  $smarty->display("view_climb.tpl");
}else{
  $smarty->display("top.tpl");
  $smarty->display("menu.tpl");
  $smarty->assign("climbs",$climbs);
  $smarty->display("generate_form.tpl");
}
$smarty->display("bottom.tpl");

?>
