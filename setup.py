from distutils.core import setup, Extension
setup(name="custom", version="1.0",
      ext_modules=[Extension("custom", ["custom/custom.c"]),
                   Extension("custom2", ["custom/custom2.c"]),
                   Extension("custom3", ["custom/custom3.c"]),
                   Extension("custom4", ["custom/custom4.c"]),
                   Extension("sublist", ["custom/sublist.c"])])
