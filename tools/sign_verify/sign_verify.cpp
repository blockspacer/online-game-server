// sign_verify.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include "CryptoPP/rsa.h"
#include "CryptoPP/sha.h"
#include "CryptoPP/osrng.h"
#include "CryptoPP/base64.h"

void sign(std::istream&);
void verify(std::istream&);
std::vector<byte> load_file(const std::string& path);
void save_file(const std::string& path, const std::string& data);

using R = CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA256>;

int main()
{
	try
	{
		// Open keyfile.
		std::cout << "enter path to key file      : ";
		std::string keyfile_path;
		std::getline(std::cin, keyfile_path);

		std::fstream keyfile;
		keyfile.open(keyfile_path.c_str(), std::ios::in);
		if (!keyfile.good())
			throw std::runtime_error("unable to open key file.\n");

		// Determine type of the key file.
		std::string line;
		std::getline(keyfile, line);
		if (line == "-----BEGIN RSA PRIVATE KEY-----")
			sign(keyfile);
		else if (line == "-----BEGIN PUBLIC KEY-----")
			verify(keyfile);
		else
			throw std::runtime_error("unrecognized key file type.\n");
	}
	catch (std::exception& ex)
	{
		std::cout << "error: " << ex.what() << "\n";
		return -1;
	}

    return 0;
}

void sign(std::istream& keyfile)
{
	// Read key contents
	std::string key_contents;
	std::string line;
	while (!keyfile.eof())
	{
		std::getline(keyfile, line);
		if (line == "-----END RSA PRIVATE KEY-----")
			break;

		key_contents += line + "\n";
	}

	// Load key material
	R::Signer signer;
	{
		CryptoPP::StringSource b64{ key_contents, true, new CryptoPP::Base64Decoder };
		signer.AccessKey().BERDecodePrivateKey(b64, false, 0);
	}

	// Read contents file
	std::string contents_filepath;
	std::cout << "enter path to contents file : ";
	std::getline(std::cin, contents_filepath);

	auto data_to_sign = load_file(contents_filepath);

	// Generate signature
	CryptoPP::AutoSeededRandomPool random;
	std::vector<byte> signature(signer.MaxSignatureLength());
	auto sig_len = signer.SignMessage(random, data_to_sign.data(), data_to_sign.size(), signature.data());

	// Encode as base64
	std::string b64_buf;
	CryptoPP::Base64Encoder b64{ new CryptoPP::StringSink{ b64_buf } };
	b64.Put(signature.data(), sig_len);
	b64.MessageEnd();

	// 
	save_file("signature.txt", b64_buf);
	std::cout << "signature saved to signature.txt\n";
}

void verify(std::istream& keyfile)
{
	// Read key contents
	std::string key_contents;
	std::string line;
	while (!keyfile.eof())
	{
		std::getline(keyfile, line);
		if (line == "-----END PUBLIC KEY-----")
			break;

		key_contents += line + "\n";
	}

	// Load key material.
	R::Verifier verifier;
	{
		CryptoPP::StringSource b64{ key_contents, true, new CryptoPP::Base64Decoder };
		verifier.AccessKey().Load(b64);
	}

	// Read contents file
	std::string contents_filepath;
	std::cout << "enter path to contents file : ";
	std::getline(std::cin, contents_filepath);

	auto data_to_verify = load_file(contents_filepath);

	// Read signature file
	std::string signature_filepath;
	std::cout << "enter path to signature file: ";
	std::getline(std::cin, signature_filepath);

	auto signature = load_file(signature_filepath);

	// Verify signature
	bool verify_result;
	{
		CryptoPP::StringSource b64{
			std::string(signature.begin(), signature.end()),
			true, new CryptoPP::Base64Decoder };

		std::vector<byte> signature(verifier.MaxSignatureLength());
		auto sig_len = b64.Get(signature.data(), signature.size());

		verify_result = verifier.VerifyMessage(data_to_verify.data(), data_to_verify.size(), signature.data(), sig_len);
	}

	std::cout << "verify result: " << (verify_result ? "succeeded" : "failed") << "\n";
}

std::vector<byte> load_file(const std::string& path)
{
	std::fstream file;
	file.open(path.c_str(), std::ios::in | std::ios::binary);
	if (!file.good())
		throw std::runtime_error("failed to open data file.");

	// determine length of the file
	file.seekg(0, std::ios::end);
	auto file_len = file.tellg();
	file.seekg(0, std::ios::beg);

	// read file contents
	std::vector<byte> contents((size_t)file_len);
	file.read((char*)contents.data(), contents.size());
	if (file.gcount() != contents.size())
		throw std::runtime_error("failed to read file contents.");

	return contents;
}

void save_file(const std::string& path, const std::string& data)
{
	std::fstream file;
	file.open(path.c_str(), std::ios::out | std::ios::binary);
	if (!file.good())
		throw std::runtime_error("failed to open target file.");

	file << data;
	file.flush();

	if (!file.good())
		throw std::runtime_error("failed to write file contents.");
}
