/*
 * storage.js - Grande QML Example
 *
 * Copyright (C) 2011  Imogen Software Carsten Valdemar Munk
 * All rights reserved.
 *
 * Author: Tom Swindell - <t.swindell@rubyx.co.uk>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the <organization>.
 * 4. Neither the name of the <organization> nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
var identifier = Grande.applicationName;
var version = Grande.applicationVersion;
var description = 'Local storage database.';
var estimatedSize = 1024 * 1024;

var __dbHandle = null;
var __dbTableName = 'localStorage'

function dbInstance() {
  if(!__dbHandle) {
    __dbHandle = openDatabaseSync(identifier, version, description, estimatedSize);
    __dbHandle.transaction(
      function(t) {
        t.executeSql('CREATE TABLE IF NOT EXISTS ' + __dbTableName + ' (key TEXT UNIQUE NOT NULL, value TEXT);');
      });
  }

  return __dbHandle;
}

function getItem(key) {
  var db = dbInstance();
  var result = '';

  db.readTransaction(
    function(t) {
      var results = t.executeSql('SELECT value FROM ' + __dbTableName + ' WHERE key=?;', [key]);
      if(results.rows.length > 0) {
        result = results.rows.item(0).value;
      }
    });

  return result;
}

function setItem(key, value) {
  var db = dbInstance();
  var result = null;

  db.transaction(
    function(t) {
      var results = t.executeSql('INSERT OR REPLACE INTO ' + __dbTableName + ' VALUES (?,?);', [key,value]);
      console.debug('Rows Affected: ' + results.rowsAffected);
      if(results.rowsAffected > 0) {
        result = true;
      } else {
        result = false;
      }
    });

  return result;
}

function removeItem(key) {
  var db = dbInstance();
  var result = null;

  db.transaction(
    function(t) {
      var results = t.executeSql('DELETE FROM ' + __dbTableName + ' WHERE key=?;', [key]);
      console.debug('Rows Affected: ' + results.rowsAffected);
      if(results.rowsAffected > 0) {
        result = true;
      } else {
        result = false;
      }
    });

  return result;
}

function clear() {
  var db = dbInstance();
  var result = null;

  db.transaction(
    function(t) {
      var results = t.executeSql('DELETE FROM ' + __dbTableName + ';', []);
      console.debug('Rows Affected: ' + results.rowsAffected);
      if(results.rowsAffected > 0) {
        result = true;
      } else {
        result = false;
      }
    });

  return result;
}

