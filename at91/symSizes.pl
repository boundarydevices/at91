use String::CRC32 ;
use URI::Escape;

$numArgs = $#ARGV + 1;
$lengthPos = 0x100 ;

foreach $argnum (0 .. $#ARGV) 
{
   my $fileName="+<" . $ARGV[$argnum];
   open(SOMEFILE, $fileName) or die( "cannot open $fileName for read/write" );
   my @prev = ();
   while(<SOMEFILE>) {               #loop as long as not EOF
      @parts = split(/ /, $_ );
      my $len = scalar @parts ;
      if( 3 == $len ){
         my $newaddr = hex @parts[0];
         my $prevaddr = hex @prev[0];
         if( 3 == scalar @prev ){
            $symSize = ($newaddr - $prevaddr);
            print sprintf( "%u\t", $symSize ) . sprintf( "%08x\t", $newaddr) . @prev[2] ;
         }
         @prev = @parts ;
      }
   }
   close(SOMEFILE);
}
