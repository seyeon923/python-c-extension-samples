# Defining Extension Types

- [Defining Extension Types](#defining-extension-types)
  - [Basic Type - custom](#basic-type---custom)
    - [`CustomObject`](#customobject)
    - [Type Object - `CustomType`](#type-object---customtype)
      - [PyVarObject_HEAD_INIT](#pyvarobject_head_init)
      - [tp_name](#tp_name)
      - [tp_basicsize & tp_itemsize](#tp_basicsize--tp_itemsize)
      - [tp_flags](#tp_flags)
      - [tp_doc](#tp_doc)
      - [tp_new](#tp_new)
    - [모듈 초기화](#모듈-초기화)
    - [Build](#build)
  - [데이터 멤버 및 메소드 추가된 타입 - custom2](#데이터-멤버-및-메소드-추가된-타입---custom2)
    - [데이터 멤버](#데이터-멤버)
    - [`Custom_dealloc`(`tp_dealloc`)](#custom_dealloctp_dealloc)
    - [`Custom_new`(`tp_new`)](#custom_newtp_new)
    - [`Custom_init`(`tp_init`)](#custom_inittp_init)
    - [멤버 메소드](#멤버-메소드)
    - [TP_FLAGS_BASETYPE](#tp_flags_basetype)
  - [멤버 타입 제한하기 - `custom3`](#멤버-타입-제한하기---custom3)
    - [Getter and Setter](#getter-and-setter)
    - [`tp_init` 업데이트(타입 체크 추가)](#tp_init-업데이트타입-체크-추가)
  - [Cyclic Garbage Collection 지원 - custom4](#cyclic-garbage-collection-지원---custom4)
    - [`Custom_traverse`(`tp_traverse`)](#custom_traversetp_traverse)
    - [`Custom_clear`(`tp_clear`)](#custom_cleartp_clear)
    - [`Custom_dealloc` 수정](#custom_dealloc-수정)
    - [`Py_TPFLAGS_HAVE_GC`](#py_tpflags_have_gc)
  - [다른타입 상속하기](#다른타입-상속하기)

## Basic Type - custom

[custom.c](custom.c)에서 가장 간단한 형태의 Extension 객체인 `Custom`을 만든다.

> 여기서 만드는 `Custom` 객체는 전통적인 방식의 *static* extension type 이라고 함.
> 힙에 할당되는 extension type을 정의하려면 [`PyType_FromSpec()`](https://docs.python.org/3/c-api/type.html#c.PyType_FromSpec)을 사용해야 된다고 함.

또한, 이 `custom.Custom` 타입은 가장 기본적인 형태로 상속될 수도 없고, 멤버도 없는 형태이다.

- CPython Runtime은 모든 Python 객체를 `PyObject*`(모든 타입의 Base Type) 변수로 봄
- `PyObject` 구조체는 객체의 reference count와 "type object"의 포인터를 가지고 있다.(type object가 파이썬 인터프리터에서 객체의 속성을 가져오거나 메소드가 호출될 때와 같은 경우에 어떤 함수를 호출해야될지에 대한 정보를 가지고 있음)

### `CustomObject`

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

### Type Object - `CustomType`

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

#### PyVarObject_HEAD_INIT

```c
PyVarObject_HEAD_INIT(NULL, 0)
```

위에서 얘기한 `ob_base` 필드를 채워주는 녀석이다. 필수 사항으로 걍 이렇게 쓰면 된다.

#### tp_name

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

#### tp_basicsize & tp_itemsize

```c
.tp_basicsize = sizeof(CustomObject),
.tp_itemsize = 0,
```

- 파이썬이 객체 생성 시 메모리를 얼마나 할당할지 알기위해 필요
- `tp_itemsize`는 variable-sized object 일때만 사용하고 그 외에는 0으로 둔다.

> **Note**: If you want your type to be subclassable from Python, and your type has the same tp_basicsize as its base type, you may have problems with multiple inheritance. A Python subclass of your type will have to list your type first in its `__bases__`, or else it will not be able to call your type’s `__new__()` method without getting an error. You can avoid this problem by ensuring that your type has a larger value for tp_basicsize than its base type does. Most of the time, this will be true anyway, because either your base type will be object, or else you will be adding data members to your base type, and therefore increasing its size.

#### tp_flags

```c
.tp_flags = Py_TPFLAGS_DEFAULT,
```

모든 타입은 `Py_TPFLAGS_DEFAULT` flag를 포함해야한다. 해당 플래그는 Python3.3 까지의 모든 멤버를 정의할 수 있도록 해준다. 추가적인 멤버가 필요하다면 OR(`|`)로 해당하는 플래그를 추가하면 된다.

#### tp_doc

```c
.tp_doc = PyDoc_STR("Custom objects"),
```

해당 타입에 대한 doc string이다.

#### tp_new

```c
.tp_new = PyType_GenericNew,
```

객체 생성이 가능하게 하려면 파이썬의 `__new__()`와 동일한 tp_new 핸들러를 제공해야한다. 여기서는 디폴트 구현인 `PyType_GenericNew`를 사용한다.

### 모듈 초기화

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

### Build

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

## 데이터 멤버 및 메소드 추가된 타입 - custom2

위에서 만든 `Custom`타입에 `first`, `last`, `number` 데이터 멤버와 `name` 멤버 메소드를 추가한 `custom2` 모듈을 만들어본다. 소스코드는 [custom2.c](custom2.c)에서 확인할 수 있다.

### 데이터 멤버

`CustomObject` C 구조체에 `first`, `last`, `number` 필드를 추가하여 `Custom` 타입에 해당 데이터 멤버를 추가한다. `first`, `last` 에는 이름, 성에 해당하는 Python string으로 사용하고, `number`은 C int를 사용한다. 업데이트된 `CustomObject` 구조체는 다음과 같이 정의한다.

```c
typedef struct {
    PyObject_HEAD
    PyObject* first;  // first name
    PyObject* last;  // last name
    int number;
} CustomObject;
```

`first`, `last`의 타입은 `PyObject*`, `number`의 타입은 `int`를 사용한 것을 알 수 있다.

`Custom` 타입의 애트리뷰트로 `CustomObject`의 `first`, `last`, `number` 변수를 노출 시키기 위해 해당 멤버에대한 정의를 담은 `PyMemberDef` 배열을 만들고 이를 `CustomType`의 `tp_mebers` 필드에 대입한다.

```c
static PyMemberDef Custom_members[] = {
    {"first", T_OBJECT_EX, offsetof(CustomObject, first), 0, "first name"},
    {"last", T_OBJECT_EX, offsetof(CustomObject, last), 0, "last name"},
    {"number", T_INT, offsetof(CustomObject, number), 0, "custom number"},
    {NULL}  // Sentienl
};

static PyTypeObject CustomType = {
    // ...
    .tp_members = Custom_members,
    // ...
};
```

`PyMemberDef`의 필드는 순서대로 member name, type, offset, access flags, documentation string 이 된다. 자세한 내용은 [여기](not_added_yet)를 참조

> 위와 같은 접근법의 문제는 `first`, `last` 의 타입을 Python string만으로 제한할 수 없다는 데에 있다.
> Python 사용자는 해당 멤버에 아무 타입의 Python 객체를 넣을 수 있고, 심지어 삭제할 수 도 있다.
> 해당 변수에 `NULL` 값을 넣으면 해당 애트리뷰트를 제거하는 것이 된다.
> 이후의 `custom3` 모듈에서는 해당 변수에 Python string 만 대입할 수 있도록 수정해볼 것이다.

### `Custom_dealloc`(`tp_dealloc`)

이제 `Custom` 객체에 관리해야 하는 객체가 있으므로 해당 객체의 할당과 해제에 더욱 주의를 기울여야 한다. 최소한 멤버의 reference count를 줄이고 해당 객체의 할당해제를 해주는 함수는 필수적으로 추가해야한다.

```c
static void Custom_dealloc(CustomObject* self) {
    Py_XDECREF(self->first);
    Py_XDECREF(self->last);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyTypeObject CustomType = {
    // ...
    .tp_dealloc = (destructor)Custom_dealloc,
    // ...
};
```

파이썬 객체인 멤버 데이터 `first`, `last`의 reference count를 감소시킬 때, `Py_DECREF()` 대신 `Py_XDECREF()`를 사용하는데, 이는 해당 멤버가 `tp_new` 를 통한 할당 중에 실패하는 등의 이유로 `NULL` 값이 들어갈 수도 있기 때문이다.(파이썬 인터프리터에서 `del` 을 통해서도 삭제가능)

> `Py_XDECREF()`는 `Py_DECREF()`와 동일한 동작을 수행하는데 추가적으로 `NULL`인지 체크를 하게 된다.
> 해당 변수가 `NULL`이 아닌 것이 확실하면 `Py_DECREF()`를, 아니라면 `Py_XDECREF()`를 사용한다.

또한, 마지막으로 타입 객체의 `tp_free`를 호출하여 메모리 할당을 해제한다. 이때, `CustomType.tp_free()` 를 직접 사용하지 않고, `Py_TYPE()` 매크로를 사용하여 타입 객체를 가져오는 데, 이는 `self`가 `CustomType` 타입이 아니고 그 파생 타입의 객체일 수도 있기 때문이다.

`tp_dealloc`은 `destructor` 타입(`void(*)(PyObject*)`) 인데, Custom_dealloc은 함수의 인자가 `PyObject*` 가 아닌 `CustomObject*` 이다.
`CustomObject`는 `PyObject`에서 파생된 클래스(`CustomObject`의 최 상단에 `PyOjbect_HEAD`를 정의함으로 메모리 앞부분에 `PyObject`와 동일한 필드를 갖음)로 C에서 다형성을 구현한 것이라고 보면 된다.

### `Custom_new`(`tp_new`)

`Custom` 객체가 생성될 때, `first`, `last`를 `NULL`이 아닌 빈 문자열로 할당하기 위해 `Custom_new` 함수를 구현하고 이를 `tp_new`에 대입한다.

```c
static PyObject* Custom_new(PyTypeObject* type, PyObject* args,
                            PyObject* kwargs) {
    CustomObject* self;
    self = (CustomObject*)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->first = PyUnicode_FromString("");
        if (self->first == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        self->last = PyUnicode_FromString("");
        if (self->last == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        self->number = 0;
    }
    return (PyObject*)self;
}

static PyTypeObject CustomType = {
    // ...
    .tp_new = Custom_new,
    // ...
};
```

`tp_new` 핸들러는 파이썬에서 `__new__()` 함수로 노출되는데, 필수로 구현해야되는 사항은 아니고 기본 구현인 `PyType_GenericNew` 함수를 사용해도 된다.

`tp_new` 핸들러는 인자로 생성되는 객체의 타입객체(`PyTypeObject*`)를 첫 번째 인자로 전달받고, 그 뒤에 어떠한 인자라도 전달받을 수 있다. 따라서, 위치 인자(`args`)와 키워드 인자(`kwargs`)를 함수 인자로 항상 갖는 모양이 된다. 하지만 일반적으로 `tp_new` 핸들러에선 이를 이용하지 않고 `tp_init`(파이썬에서 `__init__()`)에서 처리하도록한다.

> `Custom_new`에서 첫 번째 인자로 전달받는 `type`은 상속된 객체가 생성되는 경우 `CustomType`이 아닐 수 있다.

`tp_new`의 구현에서는 다음처럼 `tp_alloc` 함수를 통해 메모리 할당을 수행한다.

```c
self = (CustomObject *) type->tp_alloc(type, 0);
```

메모리 할당이 실패할 수도 있으므로 반환된 주소에 대해 `NULL`인지 확인해야한다.

> `tp_alloc` 멤버는 `PyType_Read()` 호출 시 기반클래스(`object`)에서 상속한 함수로 설정된다.

### `Custom_init`(`tp_init`)

초기화 함수 `Custom_init`을 구현하고 이를 `tp_init`에 할당한다. 해당 함수는 파이썬에서 `__init__()` 멤버함수에 노출된다.

```c
static int Custom_init(CustomObject* self, PyObject* args, PyObject* kwargs) {
    static char* kwlist[] = {"first", "last", "number", NULL};
    PyObject* first = NULL;
    PyObject* last = NULL;
    PyObject* tmp;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOi", kwlist, &first,
                                     &last, &self->number)) {
        return -1;
    }

    if (first) {
        tmp = self->first;
        Py_INCREF(first);
        self->first = first;
        Py_XDECREF(tmp);
    }
    if (last) {
        tmp = self->last;
        Py_INCREF(last);
        self->last = last;
        Py_XDECREF(tmp);
    }
    return 0;
}

static PyTypeObject CustomType = {
    // ...
    .tp_init = (initproc)Custom_init,
    // ...
};
```

`tp_init` 함수는 객체가 생성된 뒤 객체의 멤버 데이터를 초기화하는 역할을 한다. 함수 인자는 자기자신(`self`)와 위치인자(`args`)와 키워드 인자(`kwargs`)가된다.

성공시 0, 실패시 -1을 반환한다.

`tp_init` 함수는 `tp_new`와 다르게 호출된다고 보장되지는 않으며, 여러번 호출될 수도 있다.(직접 `__init__`을 호출하여)

> 객체의 멤버를 다른 값으로 재할당할 때, 할당 순서에 주의해야된다. 다음 코드는 문제가 없어 보이지만 잘못된 코드이다.
>
> ```c
> if (first) {
>    Py_XDECREF(self->first);
>    Py_INCREF(first);
>    self->first = first;
> }
> ```
>
> `first` 멤버는 어떠한 타입도 될 수 있으므로, 해당 객체의 파괴자코드가 무슨 짓을 할지 모른다. 예를들어, 다시 해당 객체의 `first` 멤버를 접근할 수도 있고, [Global Interpreter Lock](https://docs.python.org/3/glossary.html#term-global-interpreter-lock)을 해제하여 임의의 코드가 다시 해당 객체를 접근하고 수정하는 일이 발생할 수도 있다. **항상 편집증적으로 멤버의 재할당이 끝난 뒤 기존 객체의 reference count를 감소시키도록** 하자.
>
> > 물론, 위처럼 사용해도 문제가 되지 않는 조건이 있을 수 있지만, 해당 조건이 맞는지 일일이 확인하는 것보다 항상 위의 조언을 따라 코드를 작성하는 것이 맘이 편할 것 같다.

### 멤버 메소드

이름, 성에 해당하는 `first`, `second` 를 이어붙어 하나의 문자열로 반환하는 `Custom_name` 함수를 구현한 뒤 이를 `Custom.name`에 멤버 메소드로 노출시킨다.

다음은 `Custom_name`의 구현이다.

```c
static PyObject* Custom_name(CustomObject* self, PyObject* Py_UNUSED(ignore)) {
    if (self->first == NULL) {
        PyErr_SetString(PyExc_AttributeError, "No 'first' attritbute");
        return NULL;
    }
    if (self->last == NULL) {
        PyErr_SetString(PyExc_AttributeError, "No 'last' attribute");
        return NULL;
    }
    return PyUnicode_FromFormat("%S %S", self->first, self->last);
}
```

멤버 메소드는 항상 첫 번째 인자로 자기자신(`self`)을 인자로 받고, 추가로 위치인자와 키워드인자를 받을 수 있다. 여기서는 추가 인자를 받지 않도록 했으며 파이썬 코드로 보면 다음과 같다.

```python
class Custom:
    # ...
    def name(self):
        if not hasattr(self, 'first'):
            raise AttributeError("No 'first' attribute")
        
        if not hasattr(self, 'last'):
            raise AttributeError("No 'last' attribute")

        return '%s %s' % (self.first, self.second)
    # ...
```

`Custom` 클래스의 멤버 함수로 노출할 함수에 대한 정보를 `PyMethodDef` 배열에 포함한 뒤 이를 `CustomType.tp_methods`에 할당한다.

```c
static PyMethodDef Custom_methods[] = {
    {"name", (PyCFunction)Custom_name, METH_NOARGS,
     "Retrun the name, combining the first and last name"},
    {NULL}  // Sentinel
};

static PyTypeObject CustomType = {
    // ...
    .tp_methods = Custom_methods,
    // ...
};
```

`METH_NOARGS` flag는 `self`외에 인자가 없는 것을 표시한다.

### TP_FLAGS_BASETYPE

마지막으로 `Custom` 타입이 기반클래스로 쓰일 수 있도록 하기 위해서는 `CustomType.tp_flags`에 다음처럼 `Py_TPFLAGS_BASETYPE` flag 를 추가한다.

```c
static PyTypeObject CustomType = {
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
};
```

지금까지 메소드를 만들면서 해당 객체의 타입이 `CustomType`일 것(`CustomType`의 서브클래스가 될 수도 있음)이라는 가정 없이 잘 만들었으므로 위처럼 `Py_TPFLAGS_BASETYPE`을 문제없이 사용할 수 있다.

## 멤버 타입 제한하기 - `custom3`

`custom3` 모듈에서는 `custom2` 모듈에서 발전시켜 `first`, `last` 애트리뷰트에 대해 직접 접근을 없애고, getter, setter 함수를 제공하여 파이썬 문자열 외에 다른 타입을 사용할 수 없게하고 해당 애트리뷰트를 없애는 것도 못하도록 해본다. `custom3` 모듈의 소스코드는 [custom3.c](custom3.c)에서 확인할 수 있다.

### Getter and Setter

`first`, `last` 를 멤버 애트리뷰트로 노출시키는 대신, getter, setter 함수를 내보낸다.

`first` 멤버의 getter, setter 구현 `Custom_getfirst`, `Custom_setfirst`는 다음과 같다.

```c
static PyObject* Custom_getfirst(CustomObject* self, void* closure) {
    Py_INCREF(self->first);
    return self->first;
}

static int Custom_setfirst(CustomObject* self, PyObject* value, void* closure) {
    PyObject* tmp;
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete the 'first' attribute");
        return -1;
    }
    if (!PyUnicode_Check(value)) {
        PyErr_SetString(PyExc_TypeError,
                        "The 'first' attribute value must be a string");
        return -1;
    }

    tmp = self->first;
    Py_INCREF(value);
    self->first = value;
    Py_DECREF(tmp);

    return 0;
}
```

setter 함수를 보면 설정하려는 `value`를 삭제하는 경우(`NULL`로 대입)와 `value`가 파이썬 스트링이 아닌경우 exception을 발생시키는 것을 알 수 있다.

`last`에 대한 getter, setter 도 `Custom_getlast`, `Custom_setlast`라는 이름으로 위와 동일하게 구현하였다.

> `void* closure`는 setter, getter 함수 인자로 정의해줘야 하지만, 여기선 사용하지 않는다.

`Custom` 객체에 `first`, `last` 의 getter, setter를 추가하기 위해 해당하는 `PyGetSetDef` 정보를 넣어서 `PyGetSetDef` 배열을 만들어 이를 `CustomType.tp_getset`에 할당한다.

```c
static PyGetSetDef Custom_getsetters[] = {
    {"first", (getter)Custom_getfirst, (setter)Custom_setfirst, "first name",
     NULL},
    {"last", (getter)Custom_getlast, (setter)Custom_setlast, "last name", NULL},
    {NULL}  // Sentinel
};

static PyTypeObject CustomType = {
    // ...
    .tp_getset = Custom_getsetters,
    // ...
};
```

`PyGetSetDef`는 다음과 같이 생겼다.

```c
typedef struct PyGetSetDef {
    const char *name;
    getter get;
    setter set;
    const char *doc;
    void *closure;
} PyGetSetDef;
```

또한, `tp_members`에서 `first`, `last` 애트리뷰트를 제거한다.

```c
static PyMemberDef Custom_members[] = {
    {"number", T_INT, offsetof(CustomObject, number), 0, "custom number"},
    {NULL}  // Sentinel
};

static PyTypeObject CustomType = {
    // ...
    .tp_members = Custom_members,
    // ...
};
```

### `tp_init` 업데이트(타입 체크 추가)

추가로 `tp_init` 핸들러 함수(`Custom_init`)의 구현에서도 `first`, `last` 인자가 항상 Python 문자열 객체가 되도록 다음처럼 수정한다.

```c
static int Custom_init(CustomObject* self, PyObject* args, PyObject* kwargs) {
    static char* kwlist[] = {"first", "last", "number", NULL};
    PyObject* first = NULL;
    PyObject* last = NULL;
    PyObject* tmp;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|UUi", kwlist, &first,
                                     &last, &self->number)) {
        return -1;
    }

    if (first) {
        tmp = self->first;
        Py_INCREF(first);
        self->first = first;
        Py_XDECREF(tmp);
    }

    if (last) {
        tmp = self->last;
        Py_IncRef(last);
        self->last = last;
        Py_XDECREF(tmp);
    }

    return 0;
}
```

`PyArg_ParseTupleAndKeywords()` 함수에서 format string이 `"|OOi"`에서 `"|UUi"`로 변경된 것을 확인할 수 있다. 또한, 따라서, `first`, `last`가 더이상 `NULL`일 수 없으므로 `Py_XDECREF` 대신 `Py_DECREF`로 바꿀 수 있다.

> 다만, 여전히 `tp_dealloc`에서는 `Py_XDECREF`를 사용해야 하는데, `tp_new` 수행 중에 에러가 발생할 수 있기 때문이다.

## Cyclic Garbage Collection 지원 - custom4

파이썬에는 순환 참조로 인해 더 이상 필요 없지만(해당 객체에 접근할 수 없지만) reference count가 0이 아닌 객체를 식별하고 정리해주는 **Cyclic Garbage Collector(GC)** 가 존재한다.

예를 들어, 다음처럼 `list` 객체 `l`을 하나 만들고 다시 해당 리스트 객체안에 해당 객체 `l`을 담게 되면 참조 카운트가 다시 증가하면서 실제 객체는 한개이지만 참조 카운트는 2개가 되는 상태가 된다.

```python
import sys
l = []
print(f"l's reference count: {sys.getrefcount(l) - 1}") # 1
l.append(l)
print(f"l's reference count: {sys.getrefcount(l) - 1}") # 2
```

> `sys.getrefcount` 호출 시 전달받는 인자의 임시 객체가 생기면서 참조 카운트가 1 증가하게 되므로 위 코드에서는 1을 뺀 값을 출력하도록 하였다.

이때, 객체 `l`을 삭제하게 되면 더이상 객체 `l`에 접근할 수 없는상태(즉, 필요 없는 상태)가 되지만 참조카운트가 하나 줄어도 1로 0이 아니게 되어 실제로 파괴가 되지 않는다.
이때, 이러한 객체의 존재를 확인하고 이렇게 쓸모 없어진 객체를 파이썬의 **GC** 가 이를 식별하고 객체를 소멸시켜 준다.
이러한 가비지 수집은 파이썬에서 알아서 주기적으로 수행해 주지만 다음처럼 `gc.collect` 함수를 호출하여 직접 수행하도록 명령할 수도 있다.

```python
import gc
del l # "l"의 참조카운트가 1이 되고 실제로 파괴되지 않음
gc.collect() # 파이썬의 가비지 수집기가 이렇게 필요없어진 객체를 식별하여 파괴시켜줌
```

하지만 `custom.Custom` 객체의 경우 문제가 있는데, 다음처럼 `custom2.Custom` 객체를 생성한뒤 다시 제거하게 되면 `"custom2.Custom object destructed!"` 라는 메세지가 출력되면서 객체가 문제없이 파괴되는 것을 확인할 수 있다.

```python
import custom2
c = custom2.Custom()
del c # custom2.Custom object destructed!
```

> [custom2.c](custom2.c) 의 `Custom_dealloc`에 해당 로그를 남기도록 코드를 작성해둠.
>
> ```c
> static void Custom_dealloc(CustomObject* self) {
>     Py_XDECREF(self->first);
>     Py_XDECREF(self->last);
>     Py_TYPE(self)->tp_free((PyObject*)self);
> 
>     printf("custom2.Custom object destructed!\n");
> }
> ```

하지만, 다음처럼 `first` 속성에 자기자신을 다시 참조시킨 뒤 해당 객체에 대한 참조를 삭제하면 파괴가 되지 않는 것을 확인할 수 있다.

```python
c = custom2.Custom()
c.first = c
del c # c가 가르키던 객체가 파괴되지 않음!
```

다음처럼, 가비지 수집을 수행해도 `c`가 가르키던 객체가 파괴되지 않는 것을 확인할 수 있는데, 해당 `custom2.Custom` 타입이 가비지 컬렉터가 이를 탐지할 수 있도록 지원하지 않았기 때문이다.

```python
gc.collect()
```

이번 모듈 `custom4`([custom4.c](custom4.c))에서는 가비지 컬렉터가 이를 적절이 감지할 수 있도록 코드를 수정해본다.

> `custom3` 에서는 `first`, `last`에 가 파이썬 스트링만 할당할 수 있으므로 사실 이러한 순환참조가 일어날 가능 성은 없어 보인다.(누가`str` 상속해서 순환참조 넣도록 하면 될 수도 있을 것 같긴 함.)
>
> [python 문서](https://docs.python.org/3/extending/newtypes_tutorial.html#supporting-cyclic-garbage-collection)에서는 `custom3.Custom`의 서브클래스에서 새로운 애트리뷰트에 순환참조를 넣은 경우에도 GC가 제대로 이를 탐지하지 못할 것이라고 돼있는 것 같은데, 실제로 테스트 해보면 잘 파괴되는 것 같음...  
> > 파이썬 인터프리터에서 새로 추가되는 속성에 대해서는 GC가 그냥 순환참조를 탐색할 수 있는 것 같아보이긴 하는데, 정확히는 잘 모르겠음...

### `Custom_traverse`(`tp_traverse`)

먼저 GC가 해당 객체의 하위 객체들이 순환 참조를 이루는지 탐색할 수 있도록 다음처럼 순회 함수를 제공한다.

```c
static int Custom_traverse(CustomObject *self, visitproc visit, void *arg)
{
    int vret;
    if (self->first) {
        vret = visit(self->first, arg);
        if (vret != 0)
            return vret;
    }
    if (self->last) {
        vret = visit(self->last, arg);
        if (vret != 0)
            return vret;
    }
    return 0;
}
```

각각의 하위 객체(`first`, `last`)가 순환 참조를 이룰 수 있으므로, 각각의 하위 객체에 대해 순회 함수의 인자로 전달되는 `visit` 함수를 호출해줘야한다. `visit` 함수는 하위 객체와 추가적인 인자 `arg`를 인자로 전달받고, 정수를 반환한다. 이때, 정수가 0이 아니면 해당 값을 바로 반환해야 한다.

CPython에는 위 코드를 간단하게 작성할 수 있도록 `Py_VISIT` 매크로를 제공하는데, 이를 사용하면 다음처럼 간단하게 작성할 수 있다.

```c
static int Custom_traverse(CustomObject* self, visitproc visit, void* arg) {
    Py_VISIT(self->first);
    Py_VISIT(self->last);
    return 0;
}
```

> `Py_VISIT` 매크로를 사용하기 위해서 순회 함수 인자의 이름은 꼭 위처럼 `visit`, `arg`를 사용해야 한다.

`CustomType.tp_tarverse` 필드에 다음처럼 해당 함수를 할당해준다.

```c
static PyTypeObject CustomType = {
    // ...
    .tp_traverse = (traverseproc)Custom_traverse,
    // ...
};
```

### `Custom_clear`(`tp_clear`)

다음으로, 순환참조가 일어날 수 있는 하위객체들에 대해서 다음처럼 clear함수를 제공해야 된다.

```c
static int Custom_clear(CustomObject* self) {
    Py_CLEAR(self->first);
    Py_CLEAR(self->last);
    return 0;
}

static PyTypeObject CustomType = {
    // ...
    .tp_clear = (inquiry)Custom_clear,
    // ...
};
```

`Py_CLEAR` 매크로 대신 다음처럼 작성할 수도 있지만, 실수가 생길 수도 있고 훨씬 사용하기 편하므로 **항상 `Py_CLEAR`을 사용하도록 하자**

```c
PyObject *tmp;
tmp = self->first;
self->first = NULL;
Py_XDECREF(tmp);
```

> 애트리뷰트를 `NULL`로 지정하기전에 참조 카운트를 감소시키게 되면 해당 속성의 파괴자가 다시 해당 속성을 읽게되는 경우가 생길 수 있다.(특히, 순환참조가 있는 경우)

### `Custom_dealloc` 수정

`Custom_dealloc`을 수행 시 애트리뷰트를 삭제하면서 임의의 코드가 GC를 수행하도록 할 수 있으므로 이 객체가 GC에 의해 추적되지 않도록 `PyObject_GC_Untrack` 함수를 호출해 주어야 한다.

다음은 `PyObject_GC_Untrack` 과 `Custom_clear`함수를 사용하여 재 구현한 `Custom_dealloc`이다.

```c
static void Custom_dealloc(CustomObject* self) {
    PyObject_GC_UnTrack(self);
    Custom_clear(self);
    Py_TYPE(self)->tp_free((PyObject*)self);

    printf("custom4.Custom object destructed!\n");
}
```

### `Py_TPFLAGS_HAVE_GC`

마지막으로 `.tp_flags`에 `Py_TPFLAGS_HAVE_GC` flag를 더한다.

```c
static PyTypeObject CustomType = {
    // ...
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,
    // ...
};
```

> 만약 `tp_alloc` 과 `tp_free` 핸들러를 직접 구현한 경우, 이들도 GC를 지원하기 위해 수정해줘야 하지만 대부분의 경우 기본 구현을 사용한다.

## 다른타입 상속하기

다른 타입을 상속한 extension 타입을 만드는 것도 가능하다. 가장 쉬운 방법은 built-in 타입을 상속하는 것으로 built-in `list` 타입을 상속한 `SubList`를 만드는 간단한 예제를 살펴본다.

> extension 모듈의 타입간의 상속은 쉽지 않은가 봄.

`SubList`는 `list`를 그대로 상속한뒤 추가로 내부적으로 `state` 카운트 변수를 저장하고 `increment` 함수를 제공하여 해당 함수 호출 시 마다 `state` 값을 1씩 증가시키고 이를 반환하도록 하자.

```python
>>> import sublist
>>> s = sublist.SubList(range(3))
>>> s.extend(s)
>>> print(s)
[0, 1, 2, 0, 1, 2]
>>> print(s.increment())
1
>>> print(s.increment())
2
```

위 `sublist` 모듈의 소스코드는 [sublist.c](sublist.c)에서 확인할 수 있다.

`SubListObject`를 선언할 때 맨 앞부분에 `PyObject_HEAD` 대신 상속하려는 `list` 타입의 객체 struct인 `PyListObject`를 선언한다.(`PyListObject`가 `PyObject_HEAD`를 포함할 것임)

```c
typedef struct {
    PyListObject list;
    int state;
} SubListObject;
```

또한, 이렇게 `SubListObject`를 선언함으로 `SubList` 인스턴스의 경우 `PyObject*`, `PyListObject*`, `PySubListObject*`로 모두 안전하게 형변환할 수 있게 된다.

```c
static int SubList_init(SubListObject* self, PyObject* args, PyObject* kwds) {
    if (PyList_Type.tp_init((PyObject*)self, args, kwds) < 0) {
        return -1;
    }
    self->state = 0;
    return 0;
}
```

위처럼 init 함수는 기반클래스의 init 함수를 사용하도록 하였다. `tp_new`, `tp_dealloc` 함수에서도 직접 `tp_alloc`, `tp_free`를 호출하지 않고 기반클래스의 `tp_new`, `tp_dealloc` 에서 호출하도록 해야한다.

마지막으로 모듈 초기화과정에서 `PyType_Read` 호출 전에 `tp_base` 항목에 기반클래스인 `&PyList_Type`을 할당해줘야한다.(cross-compiler 문제로 초기화시 직접 할당이 안된다고 함.)

```c
PyMODINIT_FUNC PyInit_sublist(void) {
    PyObject* m;
    SubListType.tp_base = &PyList_Type;
    if (PyType_Ready(&SubListType) < 0) {
        return NULL;
    }

    m = PyModule_Create(&sublistmodule);
    if (m == NULL) {
        return NULL;
    }

    Py_INCREF(&SubListType);
    if (PyModule_AddObject(m, "SubList", (PyObject*)&SubListType) < 0) {
        Py_DECREF(&SubListType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
```
