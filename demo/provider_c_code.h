#include "structmember.h"
#include "../src/extensibletype.h"
#include "thestandard.h"

#if PY_VERSION_HEX < 0x02050000
  #define NAMESTR(n) ((char *)(n))
#else
  #define NAMESTR(n) (n)
#endif

double func(double x) {
  return x * x;
}

PyExtensibleTypeObjectEntry my_custom_slots[1] = {
  {EXTENSIBLETYPE_DOUBLE_FUNC_SLOT, func}
};

static PyObject *Provider_new(PyTypeObject *t, PyObject *a, PyObject *k) {
  PyObject *o = (*t->tp_alloc)(t, 0);
  if (!o) return 0;
  return o;
}

static void Provider_dealloc(PyObject *o) {
  (*Py_TYPE(o)->tp_free)(o);
}


typedef struct {
  PyObject_HEAD
} Provider_Object;

PyHeapExtensibleTypeObject Provider_Type;

int ProviderType_Ready(void) {
  /* Set as_number, as_buffer etc. to 0; these could of course be
     explicitly initialized too */
  memset(&Provider_Type, 0, sizeof(PyHeapExtensibleTypeObject));
  Provider_Type.etp_base.ht_type = (PyTypeObject){
    PyVarObject_HEAD_INIT(0, 0)
    NAMESTR("providertype"), /*tp_name*/
    sizeof(Provider_Object),         /* tp_basicsize */
    0,                        /* tp_itemsize */
    Provider_dealloc, /*tp_dealloc*/
    0, /*tp_print*/
    0, /*tp_getattr*/
    0, /*tp_setattr*/
#if PY_MAJOR_VERSION < 3
    0, /*tp_compare*/
#else
    0, /*reserved*/
#endif
    0, /*tp_repr*/
    &Provider_Type.etp_base.as_number, /*tp_as_number*/
    &Provider_Type.etp_base.as_sequence, /*tp_as_sequence*/
    &Provider_Type.etp_base.as_mapping, /*tp_as_mapping*/
    0, /*tp_hash*/
    0, /*tp_call*/
    0, /*tp_str*/
    0, /*tp_getattro*/
    0, /*tp_setattro*/
    &Provider_Type.etp_base.as_buffer, /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_CHECKTYPES|Py_TPFLAGS_BASETYPE, /*tp_flags*/
    0, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    0, /*tp_iter*/
    0, /*tp_iternext*/
    0, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    0, /*tp_init*/
    0, /*tp_alloc*/
    Provider_new, /*tp_new*/
    0, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    0, /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    0 /*tp_version_tag*/
#endif
  };

  PyTypeObject *extensibletype = PyExtensibleType_Import();
  if (!extensibletype) return -1;
  ((PyObject*)&Provider_Type)->ob_type = extensibletype;
  Provider_Type.etp_count = 1;
  Provider_Type.etp_custom_slots[0].id = EXTENSIBLETYPE_DOUBLE_FUNC_SLOT;
// = my_custom_slots;
  Provider_Type.etp_custom_slots[0].data = &func;
  //  {, func}

  if (PyType_Ready((PyTypeObject*)&Provider_Type) < 0) {
    return -1;
  }
  return 0;
}
