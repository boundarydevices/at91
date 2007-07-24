use String::CRC32 ;
use URI::Escape;

$numArgs = $#ARGV + 1;
$lengthPos = 0x100 ;

foreach $argnum (0 .. $#ARGV) 
{
   my $fileName="+<" . $ARGV[$argnum];
   open(SOMEFILE, $fileName) or die( "cannot open $fileName for read/write" );
   binmode(SOMEFILE);

   $crc = crc32(*SOMEFILE);

   seek (SOMEFILE, $lengthPos, SEEK_SET) or die "Can't seek: $!\n";
   my $length ;
   4 == read(SOMEFILE,$length,4) or die "Error reading file length at offset $lengthPos\n" ;
   $length = unpack("L",$length);

   seek(SOMEFILE, 0, 2);
   my $eof = tell(SOMEFILE);
   if( $eof == $length ){
      die( "length == eof == $eof\n" );
   }
   elsif( $eof == $length+4 ) {
      seek (SOMEFILE, $eof-4,SEEK_SET);
      4 == read(SOMEFILE,$crc,4) || die "Error reading old crc at eof" ;
      print( unpack("L",$crc) );
   }
   elsif( 0 == $length ){
      seek (SOMEFILE, $lengthPos, SEEK_SET) or die "Can't seek: $!\n";
      $length = pack("L",$eof); # make actual length a string
      print SOMEFILE $length ;
      seek(SOMEFILE, 0, 2); # seek EOF
      print SOMEFILE pack("L",$crc);
   }
   else {
      die( "weird length == $length, eof == $eof\n" );
   }
   print( "\n" ) if( -t STDOUT || (0 < $#ARGV) );
   close(SOMEFILE);
}

