dll <- NULL

`init_prot_env` <- NULL
`R_display_hilbert` <- NULL
`dotsapplyR` <- NULL
`R_display_hilbert_3channel` <- NULL

Hilbert.ProtEnv <- NULL   

.onLoad <- function( libname, pkgname ) {
   
   dll <<- try( library.dynam( pkgname, package=pkgname, lib.loc=libname ) )
   if( class(dll) == "try-error" )
      failedToLoadDLL( )
      
   `init_prot_env` <<- dll$`init_prot_env`
   `R_display_hilbert` <<- dll$`R_display_hilbert`
   `dotsapplyR` <<- dll$`dotsapplyR`
   `R_display_hilbert_3channel` <<- dll$`R_display_hilbert_3channel`
   
   Hilbert.ProtEnv <<- .Call( `init_prot_env` )   
}

.onUnload <- function( libpath ) {
   library.dynam.unload( dll[["name"]], libpath )
}

failedToLoadDLL <- function( ) {

   if( .Platform$pkgType != "source" ) {
   
      # binary packages used
   
      cat( sep="\n",
         "",
         " | The package's DLL could not be loaded. Most likely this is",
         " | because you do not have 'gtkmm' installed on your system,",
         " | which is required by the package." )
      
      switch( .Platform$pkgType,
         win.binary  = cat( sep="\n",
              "",
   	      " | To install gtkmm, simply download the automatic installer found",
   	      " | at the following URL:",
   	      " | http://ftp.gnome.org/pub/gnome/binaries/win32/gtkmm/2.14/gtkmm-win32-runtime-2.14.1-2.exe",
   	      " | Simply start the installer, accept all the default settings,",
   	      " | then restart R and try again to load this package.",
	      "" ),
         mac.binary =  
            cat( sep="\n",
               "",
               " | To install gtkmm, simply download the automatic install found",
               " | by copying and pasting the following URL into the address",
               " | line of your web browser (e.g., Safari):",
               " | ",
               " | http://www.ebi.ac.uk/~anders/gtkr/GTK+_2.14.X11_with_gtkmm.pkg.zip",
               " | ",
               " | The installer should start automatically. (If it does not, just ",
               " | double-click onto the downloaded file",
               " | After the installer has finished, restart R and try again ",
               " | to load this package.",
               " | ",
               " | IMPORTANT NOTE: Due to a configuration problem with the automatic",
               " | built machines, the Mac version of HilbertVisGUI is currently",
               " | broken. Please give us a few days to resolve this and then try",
               " | again. Please see this web page for status updates:",
               " | http://www.ebi.ac.uk/huber-srv/hilbert/download_R_packages.html",
               " |             [Note added 2009-March-23]",
               "" ), 
 
         stop( "Unknown value in .Platform$pkgType" ) ) 
   } else {
   
       # source packages used
       
       # In this case gtkmm must be present as the user could otherwise not have
       # managed to compile the package.

      cat( sep="\n",
         " | It seems that you installed this package from source. Hence it is odd that ",
         " | you could succesfully build the DLL and now cannot load it. Something must",
         " | have changed on your system. Maybe the error message above that was printed",
         " | by 'library.dynam' is of help. Sorry that this message is not more helpful",
         " | (it is meant for users of binary packages).",
         "" )


   }
	       	 
   cat( sep="\n",
      " | If you continue to have problems, please let me ",
      " | (sanders@fs.tum.de) know.",
      "" )
      
   stop( "Cannot load DLL. Please read the help text above." ) 	 
}
