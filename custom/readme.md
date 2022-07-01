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
