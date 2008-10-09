dll <- NULL

`init_prot_env` <- NULL
`R_display_hilbert` <- NULL
`dotsapplyR` <- NULL
`R_display_hilbert_3channel` <- NULL

Hilbert.ProtEnv <- NULL   

.onLoad <- function( libname, pkgname ) {
   
   try( dll <<- library.dynam( pkgname, package=pkgname, lib.loc=libname ) )
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

   cat( sep="\n",
      "",
      "The package's DLL could not be loaded. Most likely this is",
      "because you do not habe 'gtkmm' installed on your system,",
      "which is required by the package." )
      
   switch( .Platform$pkgType,
      win.binary  = cat( sep="\n",
	 "",
	 "To install gtkmm, simply download the automatic installer found",
	 "at the following URL:",
	 "http://ftp.gnome.org/pub/gnome/binaries/win32/gtkmm/2.14/gtkmm-win32-runtime-2.14.1-2.exe",
	 "Simply start the installer, accept all the default settings,",
	 "then restart R and try again.",
	 "" ),
      mac.binary = cat( sep="\n",
	 "",
	 "If you are using the MacPorts system (www.macports.org) or",
	 "the Fink ports system, use one of these to install the",
	 "'gtkmm' port. Then, restart R and try again.",
	 "If you do not use a ports system, please consider installing",
	 "MacPorts or wait for the GTK+ Project Team to finish their",
	 "native Mac version: http://www.gtk.org/download-macos.html",
	 "" ),
      source = {
	 switch( .Platform$OS.type,
	    unix = cat( sep="\n",
               "",
	       "On most Linux distributions and several other Unix variants,",
	       "'gtkmm' can be easily installed with the distribution's ",
	       "package manager. Have a look at the instructions at",
	       "http://www.gtkmm.org/download.shtml (under the heading 'Binary')",
	       "to see what you need to install. After you have installed gtkmm,",
	       "restart R and try again, please.",
	       "" ),
	    windows = cat( sep="\n",
               "",
	       "You are using source packages on Microsoft Windows, which is",
	       "recommended for experienced users only. If you want to continue",
	       "you can install a 'devel' package of 'gtkmm' with this automatic",
	       "installer:",
	       "http://ftp.gnome.org/pub/gnome/binaries/win32/gtkmm/2.14/gtkmm-win32-devel-2.14.1-2.exe",
	       "" ),
	       # Note that a Windows source package user should not see the above
	       # message as she cannot compile the package without having first 
	       # installed the mentioned package. Nevertheless I leave this in in
	       # case it is helpful for somebody
            error( "Unknown value in .Platform$OS.type" ) ) },
      error( "Unknown value in .Platform$pkgType" ) )
	       	 
   cat( sep="\n",
      "If you continue to have problems, please let me ",
      "(sanders@fs.tum.de) know." )
      
   error( "Cannot load DLL." ) 	 
}
