/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/qstring.h>
#include <QtCore/qvarlengtharray.h>
#include <QtCore/qdatetime.h>
#include "qjsengine.h"
#include "qjsvalue.h"
#include "qjsvalue_p.h"
#include "qv4value_p.h"
#include "qv4object_p.h"
#include "qv4functionobject_p.h"
#include "qv4dateobject_p.h"
#include "qv4runtime_p.h"
#include "qv4variantobject_p.h"
#include "qv4regexpobject_p.h"
#include "qv8engine_p.h"
#include <private/qv4mm_p.h>

QV4::Value QJSValuePrivate::getValue(QV4::ExecutionEngine *e)
{
    if (!this->engine)
        this->engine = e;
    else if (this->engine != e) {
        qWarning("JSValue can't be reassigned to another engine.");
        return QV4::Value::emptyValue();
    }
    if (value.asString() == &string) {
        value = QV4::Value::fromString(engine->newString(string.toQString()));
        PersistentValuePrivate **listRoot = &engine->memoryManager->m_persistentValues;
        prev = listRoot;
        next = *listRoot;
        *prev = this;
        if (next)
            next->prev = &this->next;
    }
    return value;
}

/*!
  \since 5.0
  \class QJSValue

  \brief The QJSValue class acts as a container for Qt/JavaScript data types.

  \ingroup qtjavascript
  \inmodule QtQml
  \mainclass

  QJSValue supports the types defined in the \l{ECMA-262}
  standard: The primitive types, which are Undefined, Null, Boolean,
  Number, and String; and the Object type. Additionally, built-in
  support is provided for Qt/C++ types such as QVariant and QObject.

  For the object-based types (including Date and RegExp), use the
  newT() functions in QJSEngine (e.g. QJSEngine::newObject())
  to create a QJSValue of the desired type. For the primitive types,
  use one of the QJSValue constructor overloads.

  The methods named isT() (e.g. isBool(), isUndefined()) can be
  used to test if a value is of a certain type. The methods named
  toT() (e.g. toBool(), toString()) can be used to convert a
  QJSValue to another type. You can also use the generic
  QJSValue_cast() function.

  Object values have zero or more properties which are themselves
  QJSValues. Use setProperty() to set a property of an object, and
  call property() to retrieve the value of a property.

  \snippet code/src_script_qjsvalue.cpp 0

  If you want to iterate over the properties of a script object, use
  the QJSValueIterator class.

  Object values have an internal \c{prototype} property, which can be
  accessed with prototype() and setPrototype().

  Function objects (objects for which isCallable()) returns true) can
  be invoked by calling call(). Constructor functions can be used to
  construct new objects by calling callAsConstructor().

  Use equals() or strictlyEquals() to compare a QJSValue to another.

  Note that a QJSValue for which isObject() is true only carries a
  reference to an actual object; copying the QJSValue will only
  copy the object reference, not the object itself. If you want to
  clone an object (i.e. copy an object's properties to another
  object), you can do so with the help of a \c{for-in} statement in
  script code, or QJSValueIterator in C++.

  \sa QJSEngine, QJSValueIterator
*/

/*!
    \enum QJSValue::SpecialValue

    This enum is used to specify a single-valued type.

    \value UndefinedValue An undefined value.

    \value NullValue A null value.
*/

QT_BEGIN_NAMESPACE

using namespace QV4;

/*!
  Constructs a new QJSValue with a boolean \a value.
*/
QJSValue::QJSValue(bool value)
    : d(new QJSValuePrivate(0, Value::fromBoolean(value)))
{
}

QJSValue::QJSValue(QJSValuePrivate *dd)
    : d(dd)
{
}

/*!
  Constructs a new QJSValue with a number \a value.
*/
QJSValue::QJSValue(int value)
    : d(new QJSValuePrivate(0, Value::fromInt32(value)))
{
}

/*!
  Constructs a new QJSValue with a number \a value.
*/
QJSValue::QJSValue(uint value)
    : d(new QJSValuePrivate(0, Value::fromUInt32(value)))
{
}

/*!
  Constructs a new QJSValue with a number \a value.
*/
QJSValue::QJSValue(double value)
    : d(new QJSValuePrivate(0, Value::fromDouble(value)))
{
}

/*!
  Constructs a new QJSValue with a string \a value.
*/
QJSValue::QJSValue(const QString& value)
    : d(new QJSValuePrivate(value))
{
}

/*!
  Constructs a new QJSValue with a special \a value.
*/
QJSValue::QJSValue(SpecialValue value)
    : d(new QJSValuePrivate(0, value == UndefinedValue ? Value::undefinedValue() : Value::nullValue()))
{
}

/*!
  Constructs a new QJSValue with a string \a value.
*/
QJSValue::QJSValue(const QLatin1String &value)
    : d(new QJSValuePrivate(value))
{
}

/*!
  Constructs a new QJSValue with a string \a value.
*/
#ifndef QT_NO_CAST_FROM_ASCII
QJSValue::QJSValue(const char *value)
    : d(new QJSValuePrivate(QString::fromLatin1(value)))
{
}
#endif

/*!
  Constructs a new QJSValue that is a copy of \a other.

  Note that if \a other is an object (i.e., isObject() would return
  true), then only a reference to the underlying object is copied into
  the new script value (i.e., the object itself is not copied).
*/
QJSValue::QJSValue(const QJSValue& other)
    : d(other.d)
{
    d->ref();
}

/*!
    Destroys this QJSValue.
*/
QJSValue::~QJSValue()
{
    d->deref();
}

/*!
  Returns true if this QJSValue is of the primitive type Boolean;
  otherwise returns false.

  \sa toBool()
*/
bool QJSValue::isBool() const
{
    return d->value.isBoolean();
}

/*!
  Returns true if this QJSValue is of the primitive type Number;
  otherwise returns false.

  \sa toNumber()
*/
bool QJSValue::isNumber() const
{
    return d->value.isNumber();
}

/*!
  Returns true if this QJSValue is of the primitive type Null;
  otherwise returns false.
*/
bool QJSValue::isNull() const
{
    return d->value.isNull();
}

/*!
  Returns true if this QJSValue is of the primitive type String;
  otherwise returns false.

  \sa toString()
*/
bool QJSValue::isString() const
{
    return d->value.isString();
}

/*!
  Returns true if this QJSValue is of the primitive type Undefined;
  otherwise returns false.
*/
bool QJSValue::isUndefined() const
{
    return d->value.isUndefined();
}

/*!
  Returns true if this QJSValue is an object of the Error class;
  otherwise returns false.
*/
bool QJSValue::isError() const
{
    Object *o = d->value.asObject();
    return o && o->asErrorObject();
}

/*!
  Returns true if this QJSValue is an object of the Array class;
  otherwise returns false.

  \sa QJSEngine::newArray()
*/
bool QJSValue::isArray() const
{
    return d->value.asArrayObject();
}

/*!
  Returns true if this QJSValue is of the Object type; otherwise
  returns false.

  Note that function values, variant values, and QObject values are
  objects, so this function returns true for such values.

  \sa QJSEngine::newObject()
*/
bool QJSValue::isObject() const
{
    return d->value.asObject();
}

/*!
  Returns true if this QJSValue can be called a function, otherwise
  returns false.

  \sa call()
*/
bool QJSValue::isCallable() const
{
    return d->value.asFunctionObject();
}

/*!
  Returns true if this QJSValue is a variant value;
  otherwise returns false.

  \sa toVariant()
*/
bool QJSValue::isVariant() const
{
    Managed *m = d->value.asManaged();
    return m ? m->as<QV4::VariantObject>() : 0;
}

/*!
  Returns the string value of this QJSValue, as defined in
  \l{ECMA-262} section 9.8, "ToString".

  Note that if this QJSValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's toString() function (and possibly valueOf()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa isString()
*/
QString QJSValue::toString() const
{
    return d->value.toQString();
}

/*!
  Returns the number value of this QJSValue, as defined in
  \l{ECMA-262} section 9.3, "ToNumber".

  Note that if this QJSValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's valueOf() function (and possibly toString()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa isNumber(), toInt(), toUInt()
*/
double QJSValue::toNumber() const
{
    QV4::ExecutionContext *ctx = d->engine ? d->engine->current : 0;
    try {
        return d->value.toNumber();
    } catch (Exception &e) {
        e.accept(ctx);
        return 0;
    }
}

/*!
  Returns the boolean value of this QJSValue, using the conversion
  rules described in \l{ECMA-262} section 9.2, "ToBoolean".

  Note that if this QJSValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's valueOf() function (and possibly toString()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa isBool()
*/
bool QJSValue::toBool() const
{
    QV4::ExecutionContext *ctx = d->engine ? d->engine->current : 0;
    try {
        return d->value.toBoolean();
    } catch (Exception &e) {
        e.accept(ctx);
        return false;
    }
}

/*!
  Returns the signed 32-bit integer value of this QJSValue, using
  the conversion rules described in \l{ECMA-262} section 9.5, "ToInt32".

  Note that if this QJSValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's valueOf() function (and possibly toString()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa toNumber(), toUInt()
*/
qint32 QJSValue::toInt() const
{
    QV4::ExecutionContext *ctx = d->engine ? d->engine->current : 0;
    try {
        return d->value.toInt32();
    } catch (Exception &e) {
        e.accept(ctx);
        return 0;
    }
}

/*!
  Returns the unsigned 32-bit integer value of this QJSValue, using
  the conversion rules described in \l{ECMA-262} section 9.6, "ToUint32".

  Note that if this QJSValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's valueOf() function (and possibly toString()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa toNumber(), toInt()
*/
quint32 QJSValue::toUInt() const
{
    QV4::ExecutionContext *ctx = d->engine ? d->engine->current : 0;
    try {
        return d->value.toUInt32();
    } catch (Exception &e) {
        e.accept(ctx);
        return 0;
    }
}

/*!
  Returns the QVariant value of this QJSValue, if it can be
  converted to a QVariant; otherwise returns an invalid QVariant.
  The conversion is performed according to the following table:

    \table
    \header \li Input Type \li Result
    \row    \li Undefined  \li An invalid QVariant.
    \row    \li Null       \li A QVariant containing a null pointer (QMetaType::VoidStar).
    \row    \li Boolean    \li A QVariant containing the value of the boolean.
    \row    \li Number     \li A QVariant containing the value of the number.
    \row    \li String     \li A QVariant containing the value of the string.
    \row    \li QVariant Object \li The result is the QVariant value of the object (no conversion).
    \row    \li QObject Object \li A QVariant containing a pointer to the QObject.
    \row    \li Date Object \li A QVariant containing the date value (toDateTime()).
    \row    \li RegExp Object \li A QVariant containing the regular expression value.
    \row    \li Array Object \li The array is converted to a QVariantList. Each element is converted to a QVariant, recursively; cyclic references are not followed.
    \row    \li Object     \li The object is converted to a QVariantMap. Each property is converted to a QVariant, recursively; cyclic references are not followed.
    \endtable

  \sa isVariant()
*/
QVariant QJSValue::toVariant() const
{
    return QV4::VariantObject::toVariant(d->value);
}

/*!
  Calls this QJSValue as a function, passing \a args as arguments
  to the function, and using the globalObject() as the "this"-object.
  Returns the value returned from the function.

  If this QJSValue is not callable, call() does nothing and
  returns an undefined QJSValue.

  Calling call() can cause an exception to occur in the script engine;
  in that case, call() returns the value that was thrown (typically an
  \c{Error} object). You can call isError() on the return value to
  determine whether an exception occurred.

  \sa isCallable(), callWithInstance(), callAsConstructor()
*/
QJSValue QJSValue::call(const QJSValueList &args)
{
    FunctionObject *f = d->value.asFunctionObject();
    if (!f)
        return QJSValue();

    ExecutionEngine *engine = d->engine;
    assert(engine);

    QVarLengthArray<Value, 9> arguments(args.length());
    for (int i = 0; i < args.size(); ++i) {
        if (!args.at(i).d->checkEngine(engine)) {
            qWarning("QJSValue::call() failed: cannot call function with argument created in a different engine");
            return QJSValue();
        }
        arguments[i] = args.at(i).d->getValue(engine);
    }

    Value result;
    QV4::ExecutionContext *ctx = engine->current;
    try {
        result = f->call(ctx, Value::fromObject(engine->globalObject), arguments.data(), arguments.size());
    } catch (Exception &e) {
        e.accept(ctx);
        result = e.value();
    }

    return new QJSValuePrivate(engine, result);
}

/*!
  Calls this QJSValue as a function, using \a instance as
  the `this' object in the function call, and passing \a args
  as arguments to the function. Returns the value returned from
  the function.

  If this QJSValue is not a function, call() does nothing
  and returns an undefined QJSValue.

  Note that if \a instance is not an object, the global object
  (see \l{QJSEngine::globalObject()}) will be used as the
  `this' object.

  Calling call() can cause an exception to occur in the script engine;
  in that case, call() returns the value that was thrown (typically an
  \c{Error} object). You can call isError() on the return value to
  determine whether an exception occurred.

  \sa call()
*/
QJSValue QJSValue::callWithInstance(const QJSValue &instance, const QJSValueList &args)
{
    FunctionObject *f = d->value.asFunctionObject();
    if (!f)
        return QJSValue();

    ExecutionEngine *engine = d->engine;
    assert(engine);

    if (!instance.d->checkEngine(engine)) {
        qWarning("QJSValue::call() failed: cannot call function with thisObject created in a different engine");
        return QJSValue();
    }

    QVarLengthArray<Value, 9> arguments(args.length());
    for (int i = 0; i < args.size(); ++i) {
        if (!args.at(i).d->checkEngine(engine)) {
            qWarning("QJSValue::call() failed: cannot call function with argument created in a different engine");
            return QJSValue();
        }
        arguments[i] = args.at(i).d->getValue(engine);
    }

    Value result;
    QV4::ExecutionContext *ctx = engine->current;
    try {
        result = f->call(ctx, instance.d->getValue(engine), arguments.data(), arguments.size());
    } catch (Exception &e) {
        e.accept(ctx);
        result = e.value();
    }

    return new QJSValuePrivate(engine, result);
}

/*!
  Creates a new \c{Object} and calls this QJSValue as a
  constructor, using the created object as the `this' object and
  passing \a args as arguments. If the return value from the
  constructor call is an object, then that object is returned;
  otherwise the default constructed object is returned.

  If this QJSValue is not a function, callAsConstructor() does
  nothing and returns an undefined QJSValue.

  Calling this function can cause an exception to occur in the
  script engine; in that case, the value that was thrown
  (typically an \c{Error} object) is returned. You can call
  isError() on the return value to determine whether an
  exception occurred.

  \sa call(), QJSEngine::newObject()
*/
QJSValue QJSValue::callAsConstructor(const QJSValueList &args)
{
    FunctionObject *f = d->value.asFunctionObject();
    if (!f)
        return QJSValue();

    ExecutionEngine *engine = d->engine;
    assert(engine);

    QVarLengthArray<Value, 9> arguments(args.length());
    for (int i = 0; i < args.size(); ++i) {
        if (!args.at(i).d->checkEngine(engine)) {
            qWarning("QJSValue::callAsConstructor() failed: cannot construct function with argument created in a different engine");
            return QJSValue();
        }
        arguments[i] = args.at(i).d->getValue(engine);
    }

    Value result;
    QV4::ExecutionContext *ctx = engine->current;
    try {
        result = f->construct(ctx, arguments.data(), arguments.size());
    } catch (Exception &e) {
        e.accept(ctx);
        result = e.value();
    }

    return new QJSValuePrivate(engine, result);
}

#ifdef QT_DEPRECATED

/*!
  \obsolete

  Returns the QJSEngine that created this QJSValue,
  or 0 if this QJSValue is invalid or the value is not
  associated with a particular engine.
*/
QJSEngine* QJSValue::engine() const
{
    QV4::ExecutionEngine *engine = d->engine;
    if (engine)
        return engine->v8Engine->publicEngine();
}

#endif // QT_DEPRECATED

/*!
  If this QJSValue is an object, returns the internal prototype
  (\c{__proto__} property) of this object; otherwise returns an
  undefined QJSValue.

  \sa setPrototype(), isObject()
*/
QJSValue QJSValue::prototype() const
{
    Object *o = d->value.asObject();
    if (!o)
        return QJSValue();
    if (!o->prototype)
        return QJSValue(NullValue);
    return new QJSValuePrivate(o->internalClass->engine, Value::fromObject(o->prototype));
}

/*!
  If this QJSValue is an object, sets the internal prototype
  (\c{__proto__} property) of this object to be \a prototype;
  if the QJSValue is null, it sets the prototype to null;
  otherwise does nothing.

  The internal prototype should not be confused with the public
  property with name "prototype"; the public prototype is usually
  only set on functions that act as constructors.

  \sa prototype(), isObject()
*/
void QJSValue::setPrototype(const QJSValue& prototype)
{
    Object *o = d->value.asObject();
    if (!o)
        return;
    if (prototype.d->value.isNull()) {
        o->prototype = 0;
        return;
    }

    Object *p = prototype.d->value.asObject();
    if (!p)
        return;
    if (o->engine() != p->engine()) {
        qWarning("QJSValue::setPrototype() failed: cannot set a prototype created in a different engine");
        return;
    }
    Object *pp = p;
    while (pp) {
        if (pp == o) {
            qWarning("QJSValue::setPrototype() failed: cyclic prototype value");
            return;
        }
        pp = pp->prototype;
    }
    o->prototype = p;
}

/*!
  Assigns the \a other value to this QJSValue.

  Note that if \a other is an object (isObject() returns true),
  only a reference to the underlying object will be assigned;
  the object itself will not be copied.
*/
QJSValue& QJSValue::operator=(const QJSValue& other)
{
    if (d == other.d)
        return *this;
    d->deref();
    d = other.d;
    d->ref();
}

/*!
  Returns true if this QJSValue is equal to \a other, otherwise
  returns false. The comparison follows the behavior described in
  \l{ECMA-262} section 11.9.3, "The Abstract Equality Comparison
  Algorithm".

  This function can return true even if the type of this QJSValue
  is different from the type of the \a other value; i.e. the
  comparison is not strict.  For example, comparing the number 9 to
  the string "9" returns true; comparing an undefined value to a null
  value returns true; comparing a \c{Number} object whose primitive
  value is 6 to a \c{String} object whose primitive value is "6"
  returns true; and comparing the number 1 to the boolean value
  \c{true} returns true. If you want to perform a comparison
  without such implicit value conversion, use strictlyEquals().

  Note that if this QJSValue or the \a other value are objects,
  calling this function has side effects on the script engine, since
  the engine will call the object's valueOf() function (and possibly
  toString()) in an attempt to convert the object to a primitive value
  (possibly resulting in an uncaught script exception).

  \sa strictlyEquals()
*/
bool QJSValue::equals(const QJSValue& other) const
{
    return __qmljs_equal(d->value, other.d->value);
}

/*!
  Returns true if this QJSValue is equal to \a other using strict
  comparison (no conversion), otherwise returns false. The comparison
  follows the behavior described in \l{ECMA-262} section 11.9.6, "The
  Strict Equality Comparison Algorithm".

  If the type of this QJSValue is different from the type of the
  \a other value, this function returns false. If the types are equal,
  the result depends on the type, as shown in the following table:

    \table
    \header \li Type \li Result
    \row    \li Undefined  \li true
    \row    \li Null       \li true
    \row    \li Boolean    \li true if both values are true, false otherwise
    \row    \li Number     \li false if either value is NaN (Not-a-Number); true if values are equal, false otherwise
    \row    \li String     \li true if both values are exactly the same sequence of characters, false otherwise
    \row    \li Object     \li true if both values refer to the same object, false otherwise
    \endtable

  \sa equals()
*/
bool QJSValue::strictlyEquals(const QJSValue& other) const
{
    return __qmljs_strict_equal(d->value, other.d->value);
}

/*!
  Returns the value of this QJSValue's property with the given \a name.
  If no such property exists, an undefined QJSValue is returned.

  If the property is implemented using a getter function (i.e. has the
  PropertyGetter flag set), calling property() has side-effects on the
  script engine, since the getter function will be called (possibly
  resulting in an uncaught script exception). If an exception
  occurred, property() returns the value that was thrown (typically
  an \c{Error} object).

  \sa setProperty(), hasProperty(), QJSValueIterator
*/
QJSValue QJSValue::property(const QString& name) const
{
    Object *o = d->value.asObject();
    if (!o)
        return QJSValue();

    ExecutionEngine *engine = d->engine;
    String *s = engine->newString(name);
    uint idx = s->asArrayIndex();
    if (idx < UINT_MAX)
        return property(idx);

    s->makeIdentifier();
    QV4::ExecutionContext *ctx = engine->current;
    try {
        QV4::Value v = o->get(s);
        return new QJSValuePrivate(engine, v);
    } catch (QV4::Exception &e) {
        e.accept(ctx);
        return new QJSValuePrivate(engine, e.value());
    }
}

/*!
  \overload

  Returns the property at the given \a arrayIndex.

  This function is provided for convenience and performance when
  working with array objects.

  If this QJSValue is not an Array object, this function behaves
  as if property() was called with the string representation of \a
  arrayIndex.
*/
QJSValue QJSValue::property(quint32 arrayIndex) const
{
    Object *o = d->value.asObject();
    if (!o)
        return QJSValue();

    ExecutionEngine *engine = d->engine;
    QV4::ExecutionContext *ctx = engine->current;
    try {
        QV4::Value v = arrayIndex == UINT_MAX ? o->get(engine->id_uintMax) : o->getIndexed(arrayIndex);
        return new QJSValuePrivate(engine, v);
    } catch (QV4::Exception &e) {
        e.accept(ctx);
        return new QJSValuePrivate(engine, e.value());
    }
}

/*!
  Sets the value of this QJSValue's property with the given \a name to
  the given \a value.

  If this QJSValue is not an object, this function does nothing.

  If this QJSValue does not already have a property with name \a name,
  a new property is created.

  \sa property(), deleteProperty()
*/
void QJSValue::setProperty(const QString& name, const QJSValue& value)
{
    Object *o = d->value.asObject();
    if (!o)
        return;

    if (!value.d->checkEngine(o->engine())) {
        qWarning("QJSValue::setProperty(%s) failed: cannot set value created in a different engine", name.toUtf8().constData());
        return;
    }

    ExecutionEngine *engine = d->engine;
    String *s = engine->newString(name);
    uint idx = s->asArrayIndex();
    if (idx < UINT_MAX) {
        setProperty(idx, value);
        return;
    }

    QV4::ExecutionContext *ctx = engine->current;
    s->makeIdentifier();
    try {
        o->put(s, value.d->getValue(engine));
    } catch (QV4::Exception &e) {
        e.accept(ctx);
    }
}

/*!
  \overload

  Sets the property at the given \a arrayIndex to the given \a value.

  This function is provided for convenience and performance when
  working with array objects.

  If this QJSValue is not an Array object, this function behaves
  as if setProperty() was called with the string representation of \a
  arrayIndex.
*/
void QJSValue::setProperty(quint32 arrayIndex, const QJSValue& value)
{
    Object *o = d->value.asObject();
    if (!o)
        return;

    ExecutionEngine *engine = d->engine;
    QV4::ExecutionContext *ctx = engine->current;
    try {
        if (arrayIndex != UINT_MAX)
            o->putIndexed(arrayIndex, value.d->getValue(engine));
        else
            o->put(engine->id_uintMax, value.d->getValue(engine));
    } catch (QV4::Exception &e) {
        e.accept(ctx);
    }
}

/*!
  Attempts to delete this object's property of the given \a name.
  Returns true if the property was deleted, otherwise returns false.

  The behavior of this function is consistent with the JavaScript
  delete operator. In particular:

  \list
  \li Non-configurable properties cannot be deleted.
  \li This function will return true even if this object doesn't
     have a property of the given \a name (i.e., non-existent
     properties are "trivially deletable").
  \li If this object doesn't have an own property of the given
     \a name, but an object in the prototype() chain does, the
     prototype object's property is not deleted, and this function
     returns true.
  \endlist

  \sa setProperty(), hasOwnProperty()
*/
bool QJSValue::deleteProperty(const QString &name)
{
    Object *o = d->value.asObject();
    if (!o)
        return false;

    ExecutionEngine *engine = d->engine;
    String *s = engine->newString(name);
    return o->deleteProperty(s);
}

/*!
  Returns true if this object has a property of the given \a name,
  otherwise returns false.

  \sa property(), hasOwnProperty()
*/
bool QJSValue::hasProperty(const QString &name) const
{
    Object *o = d->value.asObject();
    if (!o)
        return false;

    ExecutionEngine *engine = d->engine;
    String *s = engine->newIdentifier(name);
    return o->__hasProperty__(s);
}

/*!
  Returns true if this object has an own (not prototype-inherited)
  property of the given \a name, otherwise returns false.

  \sa property(), hasProperty()
*/
bool QJSValue::hasOwnProperty(const QString &name) const
{
    Object *o = d->value.asObject();
    if (!o)
        return false;

    ExecutionEngine *engine = d->engine;
    String *s = engine->newIdentifier(name);
    return o->__getOwnProperty__(s);
}

/*!
 * If this QJSValue is a QObject, returns the QObject pointer
 * that the QJSValue represents; otherwise, returns 0.
 *
 * If the QObject that this QJSValue wraps has been deleted,
 * this function returns 0 (i.e. it is possible for toQObject()
 * to return 0 even when isQObject() returns true).
 *
 * \sa isQObject()
 */
QObject *QJSValue::toQObject() const
{
    QV4::QObjectWrapper *o = d->value.as<QV4::QObjectWrapper>();
    if (!o)
        return 0;

    return o->object();
}

/*!
  Returns a QDateTime representation of this value, in local time.
  If this QJSValue is not a date, or the value of the date is NaN
  (Not-a-Number), an invalid QDateTime is returned.

  \sa isDate()
*/
QDateTime QJSValue::toDateTime() const
{
    QV4::DateObject *date = d->value.asDateObject();
    if (!date)
        return QDateTime();
    return date->toQDateTime();
}

/*!
  Returns true if this QJSValue is an object of the Date class;
  otherwise returns false.

  \sa QJSEngine::newDate()
*/
bool QJSValue::isDate() const
{
    return d->value.asDateObject();
}

/*!
  Returns true if this QJSValue is an object of the RegExp class;
  otherwise returns false.
*/
bool QJSValue::isRegExp() const
{
    return d->value.as<RegExpObject>();
}

/*!
  Returns true if this QJSValue is a QObject; otherwise returns
  false.

  Note: This function returns true even if the QObject that this
  QJSValue wraps has been deleted.

  \sa toQObject(), QJSEngine::newQObject()
*/
bool QJSValue::isQObject() const
{
    return d->value.as<QV4::QObjectWrapper>() != 0;
}

QT_END_NAMESPACE
