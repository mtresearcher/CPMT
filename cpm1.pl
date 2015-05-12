#! /usr/bin/perl

use strict;
use Getopt::Long "GetOptions";
use File::Basename;

my $debug=0;

#training data is a seguence of entries
#count W X1,\ldots,Xk: 

my(@VOC,%VOC)=();
my($MAXCODE)=1;
my(%TRAIN)=();
my (%P,%Q)=();
my (%p,%totp,%q,%totq)=();

my ($DIM)= 50;
my ($MAXITER)=10;
my ($Model)="Model.csv";

sub Encode(){
    my ($word) = @_;
    my ($code)=();
    if (!defined($code=$VOC{$word})){
	$code=($VOC{$word}=$MAXCODE++);
	$VOC[$code]=$word;
    }
    return $code;
}

sub Decode(){
    my ($code) = @_;
    die "decode: code $code is out of boundaries [0,$MAXCODE)\n" if ($code < 0 || $code >= $MAXCODE);
    return $VOC[$code];
}


sub LoadTrain(){
#load training data into a tree
while (chop($_=<STDIN>)){
    my ($c,$w,@x)=split(/ +/,$_);
    print "$w ",join(" ",@x)," count: $c\n" if $debug;
    my $cw=&Encode($w);
    
    my @cx=map &Encode($_) , @x;
    print "Encoded: $cw ",join(" ",@cx)," count $c\n" if $debug;
    next if $w=~/<\/?s>/;  #remove example if w is a separator 
    $TRAIN{join(" ",($cw,@cx))}=$c;
}};


sub ResetExpectedCounts(){
    (%p,%totp,%q,%totq)=();
};



sub ComputeExpectedCounts(){

my ($x,$w,$wx,$i,$j)=();

if (!%P){
print "Initialize parameters\n";
     for ($w=1;$w<$MAXCODE;$w++){
         for ($i=1;$i<=$DIM;$i++){
             $P{$w,$i}=1/($MAXCODE+rand(1));
             $Q{$w}->[$i]=1/($DIM+rand(1));
         } 
     }
}

print  "Compute expected counts\n";
my %den=(); # denominator
foreach $wx (keys %TRAIN){
    my ($w,@x)=split(/ /,$wx); # separting word and context
    my $count=$TRAIN{$wx}; # get the counts of the word with the context
    my $den=0; my @prod=(); 
    for ($i=1;$i<=$DIM;$i++){ # for all topics
        $prod[$i]=1;
        foreach $x (@x){$prod[$i]*=$P{$x,$i}}; # get the product of all context for all topics
        $den+=$prod[$i] * $Q{$w}->[$i]; # compute the denominator
     }
    die "den($wx)=0 \n".Dumper(@prod)  if $den==0;
    my $tmp=0;
    for ($i=1;$i<=$DIM;$i++){
                $tmp=$count * $prod[$i] * $Q{$w}->[$i]/$den;
     		foreach $x (@x){$p{$x,$i}+= $tmp; $totp{$i} += $tmp};
     		$q{$w}->[$i]+=$tmp; $totq{$w} +=$tmp;
     }
}
}

#################### MAIN 

use Data::Dumper;

&LoadTrain();

#print Dumper(%TRAIN);

#initialize parameters
(%P,%Q)=();
my $totq=0; #variable to avoid repeated access to %totq

for (my $iter=1;$iter<=$MAXITER;$iter++){

    print "Start iteration $iter\n";

    &ResetExpectedCounts();
    &ComputeExpectedCounts();

    print Dumper(%totp);
    for (my $v=1;$v<$MAXCODE;$v++){

          #print "Norm: ".&Decode($v)."\n";	

          $totq=$totq{$v};

          print "Warning: Q(".&Decode($v).")=0\n" if !$totq>0;

          for (my $i=1;$i<=$DIM;$i++){
              
	      $Q{$v}->[$i]=$q{$v}->[$i]/$totq if $totq>0;
		  
  	      $P{$v,$i} = $p{$v,$i}/ $totp{$i};

	  }
      }
}

print "Saving model\n";
open(OUT," > $Model");
for my $v (keys %Q){
	print OUT &Decode($v)." ";
        for (my $i=1;$i<=$DIM;$i++){
	   printf OUT ("%4.3f ",${$Q{$v}}[$i]);
	}
        print OUT "\n";
}
