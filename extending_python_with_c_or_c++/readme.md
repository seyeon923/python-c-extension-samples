# Extending Python with C or C++

C 코드를 작성하여 쉽게 파이썬 built-in 모듈을 만들 수 있는데, 이렇게 C로 작성된 파이썬 모듈을 **extension module** 이라고 한다.
이러한 extension module(확장모듈) 은 파이썬에서는 할 수 없는 2가지를 할 수 있는데, 첫 째는 built-in 타입을 만들 수 있다는 것과 다른 하나는 C 라이브러리 함수 호출과 system call을 사용할 수 있다는 것이다.

이러한 확장모듈을 개발하기 위해서 각종 함수, 매크로, 변수들이 정의돈 Python API가 제공되며, C 헤더파일인 `"Python.h"`를 포함하여 사용할 수 있다.

> 파이썬 헤더 파일 `"Python.h"`은 파이썬을 설치하게 되면 설치 위치의 `include` 디렉터리에 위치하며 컴파일시 해당 디렉터리를 include directory에 포함시켜야 한다.
>
> C extension interface는 [CPython](https://github.com/python/cpython) 에만 적용할 수 있으며 다른 파이썬 구현에서는 사용할 수 없다. 만약 단순히 system call이나 C 라이브러리 함수호출을 위한다면 파이썬 구현에 상관없이 사용할 수 있는 [`ctypes`](https://docs.python.org/3/library/ctypes.html#module-ctypes)모듈이나 [`cffi`](https://cffi.readthedocs.io/en/latest/)라이브리러리를 사용하도록 하자.

## A Simple Example

전달받은 시스템 명령을 수행해주는 C 라이브러리 함수 [`system`](https://en.cppreference.com/w/cpp/utility/program/system) 를 호출해주는 확장모듈 `kimchi`를 만들어본다.
파이썬에서 `kimchi` 모듈을 임포트하여 다음처럼 `system` 함수를 호출할 수 있도록 만들 것이다.

```python
>>> import kimchi
>>> status = kimchi.system('dir') # 'ls' equivalent for windows
```

먼저 [`kimchimodule.c`](kimchimodule.c) 소스파일을 만든다.

> 예전부터 모듈명이 `kimchi` 이면, 이를 구현한 C 소스파일은 `kimchimodule.c`로 사용했다고 한다. 만약, 모듈명이 `kimchijjigae` 처럼 긴 경우에는, `kimchijjigae.c` 처럼 파일명을 사용할 수 도 있다.

`kimchimodule.c`의 맨 윗부분에는 Pyuthon API를 사용할 수 있도록 다음 두 줄을 포함해준다.

```c
#define PY_SSIZE_T_CLEAN
#include <Python.h>
```

> `Python.h`에는 표준 라이브러리에 영향을 줄 수 있는 전처리기 정의 등이 있을 수 있으므로 항상 `Python.h` 헤더파일을 표준 라이브러리 헤더파일보다 앞에 위치시켜야 한다.
>
> `PY_SSIZE_T_CLEAN`을 `Python.h`를 포함하기 전에 항상 정의하는 것을 추천한다. [여기][extracting_parameters_in_extension]에서 해당 매크로에 대한 설명을 확인할 수 있다.

`Python.h`에서 정의된 모든 사용가능한 심볼은 `Py` 혹은 `PY` 가 앞에 붙어있다. 또한, 다음의 몇가지 표준 헤더들도 포함하고 있다.: `<stdio.h>`, `<string.h>`, `<errno.h>`, `<stdlib.h>`

다음으로, 파이썬에서 `kimchi.system(str)` 에 해당하는 함수 `kimchi_system` C 함수를 추가한다.

```c
static PyObject* kimchi_system(PyObject* self, PyObject* args) {
    char const* command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command)) {
        return NULL;
    }
    sts = system(command);
    return PyLong_FromLong(sts);
}
```

파이썬에서 호출되는 C 함수에는 항상 최소 2개의 인자를 갖는다. 관습적으로 `self`, `args`를 변수명으로 사용한다.

`self` 인자에는 모듈 레벨 함수인 경우에는 모듈 객체가 전달되고, 특정 타입의 메소드인 경우 해당 타입의 객체가 전달된다.

`args` 인자는 파이썬에서 호출 시 전달받은 인자를 포함하는 `tuple`을 가리키는 포인터가 된다. 예를 들어, 위에 파이썬 코드에서처럼 `kimchi.system('dir')` 처럼 호출했다면 `args`는 `('dir',)` 튜플을 가리킬 것이고, 만약 `kimchi.system(obj_a, obj_b)` 처럼 호출했다면 `args`는 `(obj_a, obj_b)` 튜플을 가르키는 변수가 될 것이다. 전달받은 튜플 객체를 C 함수에서 사용하기 위해선 이를 C 변수로 변환해야 하는데, 이 때 사용되는 함수가 `PyArg_ParseTuple` 함수이다. 이 함수는 전달받은 튜플이 포함하는 객체의 타입을 확인하고 이를 C 변수 값으로 변환해준다. 이때, 전달받은 튜플이 포함하는 객체의 타입과 변환되는 C 변수의 타입을 지정하기 위해 2번째 인자로 전달받은 템플릿 문자열을 이용한다. 해당 함수의 좀 더 자세한 설명은 [여기][extracting_parameters_in_extension]에서 확인할 수 있다.

[extracting_parameters_in_extension]: ./readme.md
