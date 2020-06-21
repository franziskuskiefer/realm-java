/*
 * Copyright 2020 Realm Inc.
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
package io.realm.log.obfuscator

import org.junit.Test
import kotlin.test.assertEquals
import kotlin.test.assertFailsWith

const val EMAIL_PASSWORD_ORIGINAL_INPUT = """{"blahblahblah":"blehblehbleh","username":"my_username","password":"123456","something":"random"}"""
const val EMAIL_PASSWORD_OBFUSCATED_OUTPUT = """{"blahblahblah":"blehblehbleh","username":"***","password":"***","something":"random"}"""

class EmailPasswordObfuscatorTest {

    @Test
    fun obfuscate() {
        EmailPasswordObfuscator.obfuscator()
                .obfuscate(EMAIL_PASSWORD_ORIGINAL_INPUT)
                .let { assertEquals(EMAIL_PASSWORD_OBFUSCATED_OUTPUT, it) }
    }

    @Test
    fun obfuscate_doesNothing() {
        EmailPasswordObfuscator.obfuscator()
                .obfuscate(IRRELEVANT_INPUT)
                .let { assertEquals(IRRELEVANT_INPUT, it) }
    }

    @Test
    fun obfuscate_fails() {
        assertFailsWith<NullPointerException> {
            EmailPasswordObfuscator.obfuscator().obfuscate(null)
        }
    }
}
