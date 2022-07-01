import glob
import sys

built_lib_path = glob.glob('build/lib*')

sys.path = built_lib_path + sys.path

print('Test module custom')
import custom

c1 = custom.Custom()
print(c1)
print()

print('Test module custom2')
import custom2

c2 = custom2.Custom('Se-yeon', 'Kim', 1)
print(f'c2.name() = {c2.name()}')
print(f'c2.first = {c2.first}')
print(f'c2.last = {c2.last}')
print(f'c2.number = {c2.number}')
print()

del c2.first

try:
    print(f'c2.name() = {c2.name()}')
except Exception as ex:
    print(f'{type(ex).__name__}: {ex}')
del c2
print()

print('Test module custom3')
import custom3

c3 = custom3.Custom('Se-yeon', 'Kim', 2)
print(f'c3.name() = {c3.name()}')
print(f'c3.first = {c3.first}')
print(f'c3.last = {c3.last}')
print(f'c3.number = {c3.number}')
print()

try:
    c3.first = 30
except Exception as ex:
    print(f'{type(ex).__name__}: {ex}')


try:
    del c3.first
except Exception as ex:
    print(f'{type(ex).__name__}: {ex}')
print()

print('Test module custom3')
import custom4
import gc


class Derived(custom3.Custom):
    pass


print('Create object "n" derived from custom3.Custom')
n = Derived()
print('Add circular reference')
n.some_attribute = n
print('Delete "n"')
del n
print('Do garbage collect')
gc.collect()
print()


class Derived(custom4.Custom):
    pass


print('Create object "n" derived from custom4.Custom')
n = Derived()
print('Add circular reference')
n.some_attribute = n
print('Delete "n"')
del n
print('Do garbage collect')
gc.collect()
print()
