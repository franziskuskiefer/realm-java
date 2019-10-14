/*
 * Copyright 2014 Realm Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "io_realm_internal_UncheckedRow.h"
#include "io_realm_internal_Property.h"

#include "java_accessor.hpp"
#include "util.hpp"

using namespace realm;
using namespace realm::_impl;

static void finalize_unchecked_row(jlong ptr);

JNIEXPORT jlong JNICALL Java_io_realm_internal_UncheckedRow_nativeGetColumnCount(JNIEnv*, jobject, jlong nativeRowPtr)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!OBJ(nativeRowPtr)->is_valid()) {
        return 0;
    }

    return static_cast<jlong>(OBJ(nativeRowPtr)->get_table()->get_column_count()); // noexcept
}

JNIEXPORT jlong JNICALL Java_io_realm_internal_UncheckedRow_nativeGetColumnKeyByName(JNIEnv* env, jobject,
                                                                                 jlong nativeRowPtr,
                                                                                 jstring columnName)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!OBJ(nativeRowPtr)->is_valid()) {
        ThrowException(env, IllegalArgument, "Object passed is not valid");
    }

    try {
        JStringAccessor columnName2(env, columnName);                                   // throws
        ColKey col_key = OBJ(nativeRowPtr)->get_table()->get_column_key(columnName2);
        if (bool(col_key)) {
            return col_key.value;
        }
    }
    CATCH_STD()
    return ColKey().value;
}

JNIEXPORT jobjectArray JNICALL Java_io_realm_internal_UncheckedRow_nativeGetColumnNames(JNIEnv* env, jobject,
                                                                               jlong nativeRowPtr)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!OBJ(nativeRowPtr)->is_valid()) {
        ThrowException(env, IllegalArgument, "Object passed is not valid");
    }

    try {
        ColKeys col_keys = OBJ(nativeRowPtr)->get_table()->get_column_keys();

        size_t size = col_keys.size();
        jobjectArray col_keys_array = env->NewObjectArray(size, JavaClassGlobalDef::java_lang_string(), 0);
        if (col_keys_array == NULL) {
            ThrowException(env, OutOfMemory, "Could not allocate memory to return column keys.");
            return NULL;
        }
        for (size_t i = 0; i < size; ++i) {
            env->SetObjectArrayElement(col_keys_array, i, to_jstring(env,  OBJ(nativeRowPtr)->get_table()->get_column_name(col_keys[i])));
        }

        return col_keys_array;

    }
    CATCH_STD()
    return NULL;
}


JNIEXPORT jint JNICALL Java_io_realm_internal_UncheckedRow_nativeGetColumnType(JNIEnv*, jobject, jlong nativeRowPtr,
                                                                               jlong columnKey)
{
    TR_ENTER_PTR(nativeRowPtr)

    ColKey column_key (columnKey);
    auto table = OBJ(nativeRowPtr)->get_table();
    jint column_type = table->get_column_type(column_key);
    if (table->is_list(column_key) && column_type < type_LinkList) {
        // add the offset so it can be mapped correctly in Java (RealmFieldType#fromNativeValue)
        column_type += 128;
    }

    return column_type;
}

JNIEXPORT jlong JNICALL Java_io_realm_internal_UncheckedRow_nativeGetColumnKey(JNIEnv* env, jobject, jlong nativeRowPtr)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return 0;
    }

    return static_cast<jlong>(OBJ(nativeRowPtr)->get_key().value);
}

JNIEXPORT jlong JNICALL Java_io_realm_internal_UncheckedRow_nativeGetLong(JNIEnv* env, jobject, jlong nativeRowPtr,
                                                                          jlong columnKey)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return 0;
    }
    ColKey col_key(columnKey);
    if (OBJ(nativeRowPtr)->get_table()->is_nullable(col_key)) {
        auto val = OBJ(nativeRowPtr)->get<util::Optional<int64_t>>(col_key);
        return val.value();
    } else {
        return  OBJ(nativeRowPtr)->get<int64_t>(col_key);
    }
}

JNIEXPORT jboolean JNICALL Java_io_realm_internal_UncheckedRow_nativeGetBoolean(JNIEnv* env, jobject,
                                                                                jlong nativeRowPtr, jlong columnKey)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return 0;
    }

    return to_jbool(OBJ(nativeRowPtr)->get<bool>(ColKey(columnKey)));
}

JNIEXPORT jfloat JNICALL Java_io_realm_internal_UncheckedRow_nativeGetFloat(JNIEnv* env, jobject, jlong nativeRowPtr,
                                                                            jlong columnKey)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return 0;
    }

    return OBJ(nativeRowPtr)->get<float>(ColKey(columnKey));
}

JNIEXPORT jdouble JNICALL Java_io_realm_internal_UncheckedRow_nativeGetDouble(JNIEnv* env, jobject,
                                                                              jlong nativeRowPtr, jlong columnKey)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return 0;
    }

    return OBJ(nativeRowPtr)->get<double>(ColKey(columnKey));
}

JNIEXPORT jlong JNICALL Java_io_realm_internal_UncheckedRow_nativeGetTimestamp(JNIEnv* env, jobject,
                                                                               jlong nativeRowPtr, jlong columnKey)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return 0;
    }

    return to_milliseconds(OBJ(nativeRowPtr)->get<Timestamp>(ColKey(columnKey)));
}

JNIEXPORT jstring JNICALL Java_io_realm_internal_UncheckedRow_nativeGetString(JNIEnv* env, jobject,
                                                                              jlong nativeRowPtr, jlong columnKey)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return nullptr;
    }

    try {
        StringData value = OBJ(nativeRowPtr)->get<StringData>(ColKey(columnKey));
        return to_jstring(env, value);
    }
    CATCH_STD()
    return nullptr;
}

JNIEXPORT jbyteArray JNICALL Java_io_realm_internal_UncheckedRow_nativeGetByteArray(JNIEnv* env, jobject,
                                                                                    jlong nativeRowPtr,
                                                                                    jlong columnKey)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return nullptr;
    }

    try {
        BinaryData bin = OBJ(nativeRowPtr)->get<BinaryData>(ColKey(columnKey));
        return JavaClassGlobalDef::new_byte_array(env, bin);
    }
    CATCH_STD()
    return nullptr;
}

JNIEXPORT jlong JNICALL Java_io_realm_internal_UncheckedRow_nativeGetLink(JNIEnv* env, jobject, jlong nativeRowPtr,
                                                                          jlong columnKey)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return 0;
    }

    ColKey col_key(columnKey);
    if (OBJ(nativeRowPtr)->is_null(col_key)) {
        return jlong(-1);
    }

    return static_cast<jlong>(OBJ(nativeRowPtr)->get<ObjKey>(col_key).value);
}

JNIEXPORT jboolean JNICALL Java_io_realm_internal_UncheckedRow_nativeIsNullLink(JNIEnv* env, jobject,
                                                                                jlong nativeRowPtr, jlong columnKey)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return 0;
    }

    return to_jbool(OBJ(nativeRowPtr)->is_null(ColKey(columnKey)));
}

JNIEXPORT void JNICALL Java_io_realm_internal_UncheckedRow_nativeSetLong(JNIEnv* env, jobject, jlong nativeRowPtr,
                                                                         jlong columnKey, jlong value)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return;
    }

    try {
        OBJ(nativeRowPtr)->set<int64_t>(ColKey(columnKey), value);
    }
    CATCH_STD()
}

JNIEXPORT void JNICALL Java_io_realm_internal_UncheckedRow_nativeSetBoolean(JNIEnv* env, jobject, jlong nativeRowPtr,
                                                                            jlong columnKey, jboolean value)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return;
    }

    try {
        OBJ(nativeRowPtr)->set(ColKey(columnKey), (value == JNI_TRUE));
    }
    CATCH_STD()
}

JNIEXPORT void JNICALL Java_io_realm_internal_UncheckedRow_nativeSetFloat(JNIEnv* env, jobject, jlong nativeRowPtr,
                                                                          jlong columnKey, jfloat value)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return;
    }

    try {
        OBJ(nativeRowPtr)->set(ColKey(columnKey), value);
    }
    CATCH_STD()
}

JNIEXPORT void JNICALL Java_io_realm_internal_UncheckedRow_nativeSetDouble(JNIEnv* env, jobject, jlong nativeRowPtr,
                                                                           jlong columnKey, jdouble value)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return;
    }

    try {
        OBJ(nativeRowPtr)->set(ColKey(columnKey), value);
    }
    CATCH_STD()
}

JNIEXPORT void JNICALL Java_io_realm_internal_UncheckedRow_nativeSetTimestamp(JNIEnv* env, jobject,
                                                                              jlong nativeRowPtr, jlong columnKey,
                                                                              jlong value)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return;
    }

    try {
        OBJ(nativeRowPtr)->set(ColKey(columnKey), from_milliseconds(value));
    }
    CATCH_STD()
}

JNIEXPORT void JNICALL Java_io_realm_internal_UncheckedRow_nativeSetString(JNIEnv* env, jobject, jlong nativeRowPtr,
                                                                           jlong columnKey, jstring value)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return;
    }

    try {
        ColKey col_key(columnKey);
        if ((value == nullptr) && !(OBJ(nativeRowPtr)->get_table()->is_nullable(col_key))) {
            ThrowNullValueException(env, OBJ(nativeRowPtr)->get_table(), ColKey(columnKey));
            return;
        }
        JStringAccessor value2(env, value); // throws
        OBJ(nativeRowPtr)->set(col_key, StringData(value2));
    }
    CATCH_STD()
}

JNIEXPORT void JNICALL Java_io_realm_internal_UncheckedRow_nativeSetByteArray(JNIEnv* env, jobject,
                                                                              jlong nativeRowPtr, jlong columnKey,
                                                                              jbyteArray value)
{
    TR_ENTER_PTR(nativeRowPtr)

    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return;
    }

    try {
        auto& obj = *reinterpret_cast<realm::Obj*>(nativeRowPtr);
        ColKey col_key(columnKey);
        if (value == nullptr && !(obj.get_table()->is_nullable(col_key))) {
            ThrowNullValueException(env, OBJ(nativeRowPtr)->get_table(), ColKey(columnKey));
            return;
        }

        JByteArrayAccessor jarray_accessor(env, value);
        obj.set(col_key, jarray_accessor.transform<BinaryData>());
    }
    CATCH_STD()
}

JNIEXPORT void JNICALL Java_io_realm_internal_UncheckedRow_nativeSetLink(JNIEnv* env, jobject, jlong nativeRowPtr,
                                                                         jlong columnKey, jlong valueObjKey)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return;
    }

    try {
        OBJ(nativeRowPtr)->set(ColKey(columnKey), ObjKey(valueObjKey));
    }
    CATCH_STD()
}

JNIEXPORT void JNICALL Java_io_realm_internal_UncheckedRow_nativeNullifyLink(JNIEnv* env, jobject, jlong nativeRowPtr,
                                                                             jlong columnKey)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return;
    }

    try {
        OBJ(nativeRowPtr)->set_null(ColKey(columnKey));
    }
    CATCH_STD()
}

//TODO rename isValid
JNIEXPORT jboolean JNICALL Java_io_realm_internal_UncheckedRow_nativeIsAttached(JNIEnv*, jobject, jlong nativeRowPtr)
{
    TR_ENTER_PTR(nativeRowPtr)
    return to_jbool(OBJ(nativeRowPtr)->is_valid());
}

JNIEXPORT jboolean JNICALL Java_io_realm_internal_UncheckedRow_nativeHasColumn(JNIEnv* env, jobject obj,
                                                                               jlong nativeRowPtr, jstring columnName)
{
    ColKey col_key (Java_io_realm_internal_UncheckedRow_nativeGetColumnKeyByName(env, obj, nativeRowPtr, columnName));
    return to_jbool(bool(col_key));
}

JNIEXPORT jboolean JNICALL Java_io_realm_internal_UncheckedRow_nativeIsNull(JNIEnv* env, jobject, jlong nativeRowPtr,
                                                                            jlong columnKey)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return JNI_FALSE;
    }

    try {
        return to_jbool(OBJ(nativeRowPtr)->is_null(ColKey(columnKey)));
    }
    CATCH_STD()
    return JNI_FALSE;
}

JNIEXPORT void JNICALL Java_io_realm_internal_UncheckedRow_nativeSetNull(JNIEnv* env, jobject, jlong nativeRowPtr,
                                                                         jlong columnKey)
{
    TR_ENTER_PTR(nativeRowPtr)
    if (!ROW_VALID(env, OBJ(nativeRowPtr))) {
        return;
    }
    if (!COL_NULLABLE(env, OBJ(nativeRowPtr)->get_table(), columnKey)) {//TODO do we need the const_cast?
        return;
    }
    try {
        OBJ(nativeRowPtr)->set_null(ColKey(columnKey));
    }
    CATCH_STD()
}

static void finalize_unchecked_row(jlong ptr)
{
    TR_ENTER_PTR(ptr)
    delete OBJ(ptr);
}

JNIEXPORT jlong JNICALL Java_io_realm_internal_UncheckedRow_nativeGetFinalizerPtr(JNIEnv*, jclass)
{
    TR_ENTER()
    return reinterpret_cast<jlong>(&finalize_unchecked_row);
}
