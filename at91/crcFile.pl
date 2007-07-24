use String::CRC32 ;

$numArgs = $#ARGV + 1;

foreach $argnum (0 .. $#ARGV) 
{
   my $fileName=$ARGV[$argnum];
   open(SOMEFILE, $fileName) || die( $fileName );
   $crc = crc32(*SOMEFILE);
   print( $crc );
   print( "\n" ) if( -t STDOUT || (0 < $#ARGV) );
}


