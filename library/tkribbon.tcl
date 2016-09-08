namespace eval ::tkribbon {
  
  namespace eval platform {}

  proc ribbon {pathName args} {
    set toplevel {}
    set options {}
    foreach {option value} $args {
      switch -exact -- $option {
        -toplevel {set toplevel $value}
        default   {lappend options $option $value}
      }
    }
    if {$toplevel eq ""} {
      ## Try to guess the toplevel...
      set parent [join [lrange [split $pathName .] 0 end-1] .]
      if {$parent eq ""} {set parent .}
      set toplevel [winfo toplevel $parent]
    }
    set w [platform::create $pathName $toplevel {*}$options]
    $w attach
    return $w
  };# ribbon

};# ::tkribbon namespace 
