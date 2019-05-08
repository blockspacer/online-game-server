// stdafx.cpp : source file that includes just the standard includes
// soci_lib.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

#include "soci.h"
#include "soci/backends/sqlite/soci-sqlite3.h"

#include <iostream>
using namespace soci;
int main()
{
	session session_(std::move(soci::sqlite3), "master1.db");
	std::string user_id;
	session_ << "SELECT user_id FROM login_tb WHERE id = 1", into(user_id);

	std::cout << user_id << std::endl;
	return 0;
}