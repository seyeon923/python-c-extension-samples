from distutils.core import setup, Extension
setup(name="custom", version="1.0",
      ext_modules=[Extension("custom", ["defining_extension_types/custom.c"]),
                   Extension(
                       "custom2", ["defining_extension_types/custom2.c"]),
                   Extension(
                       "custom3", ["defining_extension_types/custom3.c"]),
                   Extension("custom4", ["defining_extension_types/custom4.c"])])

setup(name="sublist", version="1.0",
      ext_modules=[Extension("sublist", ["defining_extension_types/sublist.c"])])

setup(name="spam", version="1.0",
      ext_modules=[Extension("spam", ["extending_python_with_c_or_c++/spammodule.c"])])
