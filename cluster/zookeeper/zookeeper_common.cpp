/*  
 * Copyright (c) 2009 Carnegie Mellon University. 
 *     All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing,
 *  software distributed under the License is distributed on an "AS
 *  IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 *  express or implied.  See the License for the specific language
 *  governing permissions and limitations under the License.
 *
 * For more about this software visit:
 *
 *      http://www.graphlab.ml.cmu.edu
 *
 */


#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <boost/algorithm/string/join.hpp>
#include "zookeeper_common.hpp"

extern "C" {
#include <zookeeper.h>
}



//namespace graphlab{
//namespace zookeeper {


std::string trim(const std::string& str) {
  std::string::size_type pos1 = str.find_first_not_of(" \t");
  std::string::size_type pos2 = str.find_last_not_of(" \t");
  return str.substr(pos1 == std::string::npos ? 0 : pos1,
                    pos2 == std::string::npos ? str.size()-1 : pos2-pos1+1);
}

/**
 Using splitchars as delimiters, splits the string into a vector of strings.
 if auto_trim is true, trim() is called on all the extracted strings
 before returning.
 */
std::vector<std::string> strsplit(const std::string& str,
                                  const std::string& splitchars,
                                  const bool auto_trim = false) {
  std::vector<std::string> tokens;
  for(size_t beg = 0, end = 0; end != std::string::npos; beg = end+1) {
    end = str.find_first_of(splitchars, beg);
    if(auto_trim) {
      if(end - beg > 0) {
        std::string tmp = trim(str.substr(beg, end - beg));
        if(!tmp.empty()) tokens.push_back(tmp);
      }
    } else tokens.push_back(str.substr(beg, end - beg));
  }
  return tokens;
}

// frees a zookeeper String_vector
void free_String_vector(struct String_vector* strings) {
  if (strings->data) {
    for (size_t i = 0;i < (size_t)(strings->count); ++i) {
      free(strings->data[i]);
    }
    free(strings->data);
    strings->data = NULL;
    strings->count = 0;
  }
}

// convert a zookeeper String_vector to a c++ vector<string>
std::vector<std::string> String_vector_to_vector(
    const struct String_vector* strings) {
  std::vector<std::string> ret;
  for (size_t i = 0;i < (size_t)(strings->count); ++i) {
    ret.push_back(strings->data[i]);
  }
  return ret;
}



/// convert a zookeeper String_vector to a c++ vector<std::pair<std:string, bool>>
  std::vector< std::pair<std::string, bool> > String_vector_to_pair_vector(
    const struct String_vector* strings) {
  std::vector< std::pair<std::string, bool> > ret;
  for (size_t i = 0;i < (size_t)(strings->count); ++i) {
    char* tmp = strings->data[i];
    size_t tmp_len = strlen(tmp);
    ret.push_back( std::make_pair(std::string(tmp, tmp_len - 1),
                                  atoi(tmp + (tmp_len - 1)) != 0));
  }
  return ret;
}
  
  /// convert a zookeeper String_vector to a c++ vector<std::pair<size_t, bool>>
std::vector< std::pair<size_t, std::string> > String_vector_to_pair_vector_int(const struct String_vector* strings) {
    std::vector< std::pair<size_t, std::string> > ret;
    for (size_t i = 0;i < (size_t)(strings->count); ++i) {
      char* tmp = strings->data[i];
      std::vector<std::string> split_state = strsplit(tmp, ",");
      ret.push_back( std::make_pair(atoi(split_state[0].c_str()), split_state[1]));
      //size_t tmp_len = strlen(tmp);
      //ret.push_back( std::make_pair(atoi(std::string(tmp, tmp_len - 1).c_str()),
      //                              atoi(tmp + (tmp_len - 1)) != 0));
    }
    return ret;
  }

// print a few zookeeper error status
void print_stat(int stat,
                const std::string& prefix,
                const std::string& path) {
  if (stat == ZNONODE) {
    std::cerr << prefix << ": Node missing" << path << std::endl;
  }
  else if (stat == ZNOAUTH) {
    std::cerr << prefix << ": No permission to list children of node "
              << path << std::endl;
  }
  else if (stat == ZNODEEXISTS) {
    std::cerr << prefix << ": Node " << path << " already exists." << std::endl;
  }
  else if (stat == ZNOTEMPTY) {
    std::cerr << prefix << ": Node " << path << " not empty." << std::endl;
  }
  else if (stat != ZOK) {
    std::cerr << prefix << ": Unexpected error " << stat
              << " on path " << path << std::endl;
  }
}

// adds a trailing / to the path name if there is not one already
std::string normalize_path(std::string prefix) {
  trim(prefix);
  if (prefix.length() == 0) return "/";
  else if (prefix[prefix.length() - 1] != '/') return prefix + "/";
  else return prefix;
}

int create_dir(zhandle_t* handle, const std::string& name,
               const std::string& stat_message) {
  int stat = zoo_create(handle, name.c_str(), NULL, -1,
                       &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
  // we are ok with ZNODEEXISTS
  // if (stat == ZOK || stat == ZNODEEXISTS) return stat;
  if (stat != ZOK) print_stat(stat, stat_message + " create_dir", name);
  return stat;
}

int delete_dir(zhandle_t* handle, const std::string& name,
               const std::string& stat_message) {
  int stat = zoo_delete(handle, name.c_str(), -1);
  // we are ok if the node is not empty in which case
  // there are still machines in the name space
  // if (stat == ZOK || stat == ZNOTEMPTY) return stat;
  if (stat != ZOK) print_stat(stat, stat_message + " delete_dir", name);
  return stat;
}

int create_ephemeral_node(zhandle_t* handle,
                 const std::string& path,
                 const std::string& value,
                 const std::string& stat_message) {
  int stat = zoo_create(handle, path.c_str(), value.c_str(), value.length(),
                        &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0);
  // if (stat == ZOK) return stat;
  if (stat != ZOK) print_stat(stat, stat_message + " create_ephemeral_node", path);
  return stat;
}

int delete_node(zhandle_t* handle,
                           const std::string& path,
                           const std::string& stat_message) {
  int stat = zoo_delete(handle, path.c_str(), -1);
  //  if (stat == ZOK) return stat;
  if (stat != ZOK) print_stat(stat, stat_message + " delete_node", path);
  return stat;
}


std::string get_sequence_node_path(const std::string& path,
                                   const int version) {
  char versionstring[16];
  sprintf(versionstring, "%010d", version);
  std::string actualpath = path + versionstring;
  return actualpath;
}

int delete_sequence_node(zhandle_t* handle,
                         const std::string& path,
                         const int version,
                         const std::string& stat_message) {
  std::string actualpath = get_sequence_node_path(path, version);
  int stat = zoo_delete(handle, actualpath.c_str(), -1);
  //  if (stat == ZOK) return stat;
  if (stat != ZOK) print_stat(stat, stat_message + " delete_sequence_node", actualpath);
  return stat;
}

std::pair<int,int> create_ephemeral_sequence_node(zhandle_t* handle,
                                                  const std::string& path,
                                                  const std::string& value,
                                                  const std::string& stat_message) {
  // make sure we always have enough room for the version number
  assert(path.length() + 10 < 1024);
  char retpathbuffer[1024];
  int stat = zoo_create(handle, path.c_str(), value.c_str(), value.length(),
                        &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE,
                        retpathbuffer, 1024);
  // if (stat == ZOK) return stat;
  if (stat != ZOK) print_stat(stat, stat_message + " create_ephemeral_sequence_node", path);
  int retlen = strlen(retpathbuffer);
  assert(retlen > 10);
  int version = atoi(retpathbuffer + (retlen - 10));
  return std::pair<int, int>(stat, version);
}


std::pair<bool, std::string> get_node_value(zhandle_t* handle,
                                            const std::string& node,
                                            const std::string& stat_message) {
  char buffer[1024];
  int length = 1024;
  int stat = zoo_get(handle, node.c_str(), 0, buffer, &length, NULL);
  if (stat != ZOK) print_stat(stat, stat_message + " get_node_value", node);
  if (stat != ZOK) return std::pair<bool, std::string>(false, "");

  // we are good here
  if (length <= 1024) {
    // ok. it fit inside the buffer
    // we can return
    if (length < 0) return std::pair<bool, std::string>(true, "");
    else return std::pair<bool, std::string>(true, std::string(buffer, length));
  }
  else {
    while(1) {
      // buffer not long enough. The length parameter constains the actual length
      // try again. keep looping until we succeed
      char* newbuffer = new char[length];
      int stat = zoo_get(handle, node.c_str(), 0, newbuffer, &length, NULL);
      if (stat != ZOK) print_stat(stat, stat_message + " get_node_value", node);
      std::string retval(newbuffer, length);
      delete newbuffer;

      if (stat != ZOK) print_stat(stat, stat_message + " get_node_value", node);
      if (stat != ZOK) return std::pair<bool, std::string>(false, "");
      if (length < 0) return std::pair<bool, std::string>(true, "");
      else return std::pair<bool, std::string>(true, retval);
    }
  }
}


//} // graphlab
//} // zookeeper

