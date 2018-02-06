{
  "targets": [
    {
      "target_name": "thorden",
      "sources": [ "main.cpp", "include/log.cpp" ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      "xcode_settings": {
	      'OTHER_CFLAGS': [ '-fexceptions' ]
      }
    }
  ]
}
