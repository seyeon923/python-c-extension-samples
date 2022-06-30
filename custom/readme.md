# Basic Type

[custom.c](custom.c)에서 가장 간단한 형태의 Extension 객체인 `Custom`을 만든다.

> 여기서 만드는 `Custom` 객체는 전통적인 방식의 *static* extension type 이라고 함.
> 힙에 할당되는 extension type을 정의하려면 [`PyType_FromSpec()`](https://docs.python.org/3/c-api/type.html#c.PyType_FromSpec)을 사용해야 된다고 함.

또한, 이 `custom.Custom` 타입은 가장 기본적인 형태로 상속될 수도 없고, 멤버도 없는 형태이다.

- CPython Runtime은 모든 Python 객체를 `PyObject*`(모든 타입의 Base Type) 변수로 봄
- `PyObject` 구조체는 객체의 reference count와 "type object"의 포인터를 가지고 있다.(type object가 파이썬 인터프리터에서 객체의 속성을 가져오거나 메소드가 호출될 때와 같은 경우에 어떤 함수를 호출해야될지에 대한 정보를 가지고 있음)

## `CustomObject`

각각의 `Custom` 인스턴스에 대해 한번씩 할당되는 **object** 구조체

```c
typedef struct {
    PyObject_HEAD
} CustomObject;
```

- 모든 객체 구조체의 시작에는 `PyObject_HEAD` 를 반드시 포함해야된다.(`PyObject` 타입의 reference count와 type object 포인터를 포함하는 `ob_base` 필드를 정의해줌)

> reference count는 `Py_REFCNT`, type object pointer는 `Py_TYPE` 매크로로 접근할 수 있다.

당연히 일반적으로, `PyObject_HEAD` 뒤에 추가적인 객체의 멤버 데이터를 추가한다. 예를들어 파이썬의 float는 다음과 같이 정의할 수 있다.

```c
typedef struct {
    PyObject_HEAD
    double ob_fval;
} PyFloatObject;
```

## Type Object - `CustomType`

해당 타입에 대해 특정한 연산/동작이 요청될 때 인터프리터가 조사하는 함수 포인터들과 일련의 플래그들이 정의됨

```c
static PyTypeObject CustomType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "custom.Custom",
    .tp_doc = PyDoc_STR("Custom objects"),
    .tp_basicsize = sizeof(CustomObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
};
```

> `PyTypeObject`는 object.h에 선언되어 있으며, 위에 사용된 것보다 더 많은 필드들이 있지만, 필요한 놈들만 세팅해주면 된다.
> 또한, 필드 선언 순서 알필요 없이 지정할 수 있도록 **C99 스타일의 designated initializer** 을 사용하자!

### PyVarObject_HEAD_INIT

```c
PyVarObject_HEAD_INIT(NULL, 0)
```

위에서 얘기한 `ob_base` 필드를 채워주는 녀석이다. 필수 사항으로 걍 이렇게 쓰면 된다.

### tp_name

```c
.tp_name = "custom.Custom",
```

해당 타입의 이름으로 에러메세지 등이 표시될 때 나타나는 디폴트 텍스트 표기이다. 예를들어, 다음처럼 표기될 때 사용된다.

```interpreter
>>> "" + custom.Custom()
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
TypeError: can only concatenate str (not "custom.Custom") to str
```

> 해당 필드는 \<모듈명>.\<타입명> 처럼 **dotted name** 을 사용한다.

### tp_basicsize & tp_itemsize

```c
.tp_basicsize = sizeof(CustomObject),
.tp_itemsize = 0,
```

- 파이썬이 객체 생성 시 메모리를 얼마나 할당할지 알기위해 필요
- `tp_itemsize`는 variable-sized object 일때만 사용하고 그 외에는 0으로 둔다.

> **Note**: If you want your type to be subclassable from Python, and your type has the same tp_basicsize as its base type, you may have problems with multiple inheritance. A Python subclass of your type will have to list your type first in its `__bases__`, or else it will not be able to call your type’s `__new__()` method without getting an error. You can avoid this problem by ensuring that your type has a larger value for tp_basicsize than its base type does. Most of the time, this will be true anyway, because either your base type will be object, or else you will be adding data members to your base type, and therefore increasing its size.

### tp_flags

```c
.tp_flags = Py_TPFLAGS_DEFAULT,
```

모든 타입은 `Py_TPFLAGS_DEFAULT` flag를 포함해야한다. 해당 플래그는 Python3.3 까지의 모든 멤버를 정의할 수 있도록 해준다. 추가적인 멤버가 필요하다면 OR(`|`)로 해당하는 플래그를 추가하면 된다.

### tp_doc

```c
.tp_doc = PyDoc_STR("Custom objects"),
```

해당 타입에 대한 doc string이다.

### tp_new

```c
.tp_new = PyType_GenericNew,
```

객체 생성이 가능하게 하려면 파이썬의 `__new__()`와 동일한 tp_new 핸들러를 제공해야한다. 여기서는 디폴트 구현인 `PyType_GenericNew`를 사용한다.

## 모듈 초기화

모듈의 초기화는 [custom.c](custom.c)에서 정의한 `PyInit_custom()` 함수에서 수행하게 된다.

`PyType_Ready()` 함수를 통해 `CustomType`의 설정하지 않은 필드를 적절한 디폴트 값으로 설정하여 초기화를 하게된다.

```c
if (PyType_Ready(&CustomType) < 0) {
    return NULL;
}
```

다음은 모듈의 dictionary에 `Custom` 클래스 `Custom` 객체를 추가하는 코드이다.

```c
Py_IncRef((PyObject*)&CustomType);
if (PyModule_AddObject(m, "Custom", (PyObject*)&CustomType) < 0) {
    Py_DecRef((PyObject*)&CustomType);
    Py_DecRef(m);
    return NULL;
}
```

## Build

다음처럼 setup.py를 작성한다.

```python
from distutils.core import setup, Extension
setup(name="custom", version="1.0",
      ext_modules=[Extension("custom", ["custom.c"])])
```

다음 명령어를 통해 python extension library를 만들 수 있다.

```sh
python setup.py build
```

> 위에서 보여주는 disutils를 사용한 방식은 구식으로, setuptools 를 사용하는 것이 더 좋다.
