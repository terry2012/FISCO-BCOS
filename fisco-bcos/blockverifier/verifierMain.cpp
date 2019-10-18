/*
 * @CopyRight:
 * FISCO-BCOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FISCO-BCOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FISCO-BCOS.  If not, see <http://www.gnu.org/licenses/>
 * (c) 2016-2018 fisco-dev contributors.
 */

/**
 * @brief : verifierMain
 * @author: mingzhenliu
 * @date: 2018-09-21
 */

#include <leveldb/db.h>
#include <libblockchain/BlockChainImp.h>
#include <libblockverifier/BlockVerifier.h>
#include <libblockverifier/Common.h>
#include <libblockverifier/ExecutiveContextFactory.h>
#include <libdevcore/BasicLevelDB.h>
#include <libdevcrypto/Common.h>
#include <libethcore/Block.h>
#include <libethcore/TransactionReceipt.h>
#include <libmptstate/MPTStateFactory.h>
#include <libstorage/LevelDBStorage.h>
#include <libstorage/MemoryTableFactory.h>
#include <libstorage/Storage.h>
#include <libstoragestate/StorageStateFactory.h>

using namespace dev;

int main(int argc, char* argv[])
{
    auto storagePath = std::string("test_storage/");
    boost::filesystem::create_directories(storagePath);
    leveldb::Options option;
    option.create_if_missing = true;
    option.max_open_files = 1000;
    dev::db::BasicLevelDB* dbPtr = NULL;
    leveldb::Status s = dev::db::BasicLevelDB::Open(option, storagePath, &dbPtr);
    if (!s.ok())
    {
        LOG(ERROR) << "Open storage db error: " << s.ToString();
    }

    auto storageDB = std::shared_ptr<dev::db::BasicLevelDB>(dbPtr);
    auto storage = std::make_shared<dev::storage::LevelDBStorage>();
    storage->setDB(storageDB);

    auto blockChain = std::make_shared<dev::blockchain::BlockChainImp>();
    blockChain->setStateStorage(storage);
    dev::blockchain::GenesisBlockParam initParam = {
        "std", dev::h512s(), dev::h512s(), "", "", "", 1000, 300000000, 0, -1, -1};
    blockChain->checkAndBuildGenesisBlock(initParam);

    auto stateFactory = std::make_shared<dev::storagestate::StorageStateFactory>(dev::u256(0));

    // auto stateFactory = std::make_shared<dev::mptstate::MPTStateFactory>(
    //    dev::u256(0), "test_state", dev::h256(0), dev::WithExisting::Trust);

    auto executiveContextFactory = std::make_shared<dev::blockverifier::ExecutiveContextFactory>();
    executiveContextFactory->setStateFactory(stateFactory);
    executiveContextFactory->setStateStorage(storage);


    auto blockVerifier = std::make_shared<dev::blockverifier::BlockVerifier>();
    blockVerifier->setExecutiveContextFactory(executiveContextFactory);
    blockVerifier->setNumberHash(
        [blockChain](int64_t num) { return blockChain->getBlockByNumber(num)->headerHash(); });

    if (argc > 1 && std::string("insert") == argv[1])
    {
        for (int i = 0; i < 2; ++i)
        {
            auto max = blockChain->number();
            auto parentBlock = blockChain->getBlockByNumber(max);
            dev::eth::BlockHeader header;
            header.setNumber(max + 1);
            header.setParentHash(parentBlock->headerHash());
            header.setGasLimit(dev::u256(1024 * 1024 * 1024));
            header.setRoots(parentBlock->header().transactionsRoot(),
                parentBlock->header().receiptsRoot(), parentBlock->header().stateRoot());
            dev::eth::Block block;
            block.setBlockHeader(header);
            LOG(INFO) << "max " << max << " parentHeader " << parentBlock->header() << " header "
                      << header;

            dev::bytes rlpBytes = dev::fromHex(
                "0xf92027a0039d7614c185b85512a00fcbb2e9012dac3869fccca2c345e8d10d4fba42b02d8401c9c3"
                "808401c9c3808201f48080b91fb16060604052341561000c57fe5b5b611f958061001c6000396000f3"
                "0060606040526000357c01000000000000000000000000000000000000000000000000000000009004"
                "63ffffffff168063487a5a1014610067578063c4f41ab314610121578063ebf3b24f14610198578063"
                "efc81a8c14610252578063fcd7e3c114610264575bfe5b341561006f57fe5b61010b60048080359060"
                "2001908201803590602001908080601f01602080910402602001604051908101604052809392919081"
                "8152602001838380828437820191505050505050919080359060200190919080359060200190820180"
                "3590602001908080601f01602080910402602001604051908101604052809392919081815260200183"
                "8380828437820191505050505050919050506103cd565b604051808281526020019150506040518091"
                "0390f35b341561012957fe5b610182600480803590602001908201803590602001908080601f016020"
                "8091040260200160405190810160405280939291908181526020018383808284378201915050505050"
                "5091908035906020019091905050610a5e565b6040518082815260200191505060405180910390f35b"
                "34156101a057fe5b61023c600480803590602001908201803590602001908080601f01602080910402"
                "6020016040519081016040528093929190818152602001838380828437820191505050505050919080"
                "3590602001909190803590602001908201803590602001908080601f01602080910402602001604051"
                "908101604052809392919081815260200183838082843782019150505050505091905050610f09565b"
                "6040518082815260200191505060405180910390f35b341561025a57fe5b6102626114dd565b005b34"
                "1561026c57fe5b6102bc600480803590602001908201803590602001908080601f0160208091040260"
                "2001604051908101604052809392919081815260200183838082843782019150505050505091905050"
                "611618565b604051808060200180602001806020018481038452878181518152602001915080519060"
                "20019060200280838360008314610316575b8051825260208311156103165760208201915060208101"
                "90506020830392506102f2565b50505090500184810383528681815181526020019150805190602001"
                "9060200280838360008314610366575b80518252602083111561036657602082019150602081019050"
                "602083039250610342565b505050905001848103825285818151815260200191508051906020019060"
                "2002808383600083146103b6575b8051825260208311156103b6576020820191506020810190506020"
                "83039250610392565b505050905001965050505050505060405180910390f35b600060006000600060"
                "00600061100194508473ffffffffffffffffffffffffffffffffffffffff1663c184e0ff6000604051"
                "602001526040518163ffffffff167c0100000000000000000000000000000000000000000000000000"
                "0000000281526004018080602001828103825260068152602001807f745f7465737400000000000000"
                "0000000000000000000000000000000000000081525060200191505060206040518083038160008780"
                "3b151561048357fe5b6102c65a03f1151561049157fe5b5050506040518051905093508373ffffffff"
                "ffffffffffffffffffffffffffffffff166313db93466000604051602001526040518163ffffffff16"
                "7c01000000000000000000000000000000000000000000000000000000000281526004018090506020"
                "60405180830381600087803b151561050957fe5b6102c65a03f1151561051757fe5b50505060405180"
                "51905092508273ffffffffffffffffffffffffffffffffffffffff1663e942b516886040518263ffff"
                "ffff167c01000000000000000000000000000000000000000000000000000000000281526004018080"
                "60200180602001838103835260098152602001807f6974656d5f6e616d650000000000000000000000"
                "0000000000000000000000008152506020018381038252848181518152602001915080519060200190"
                "808383600083146105f2575b8051825260208311156105f25760208201915060208101905060208303"
                "92506105ce565b505050905090810190601f16801561061e5780820380516001836020036101000a03"
                "1916815260200191505b509350505050600060405180830381600087803b151561063a57fe5b6102c6"
                "5a03f1151561064857fe5b5050508373ffffffffffffffffffffffffffffffffffffffff16637857d7"
                "c96000604051602001526040518163ffffffff167c0100000000000000000000000000000000000000"
                "000000000000000000028152600401809050602060405180830381600087803b15156106b757fe5b61"
                "02c65a03f115156106c557fe5b5050506040518051905091508173ffffffffffffffffffffffffffff"
                "ffffffffffff1663cd30a1d18a6040518263ffffffff167c0100000000000000000000000000000000"
                "000000000000000000000000028152600401808060200180602001838103835260048152602001807f"
                "6e616d6500000000000000000000000000000000000000000000000000000000815250602001838103"
                "8252848181518152602001915080519060200190808383600083146107a0575b805182526020831115"
                "6107a05760208201915060208101905060208303925061077c565b505050905090810190601f168015"
                "6107cc5780820380516001836020036101000a031916815260200191505b5093505050506000604051"
                "80830381600087803b15156107e857fe5b6102c65a03f115156107f657fe5b5050508173ffffffffff"
                "ffffffffffffffffffffffffffffff1663e44594b9896040518263ffffffff167c0100000000000000"
                "0000000000000000000000000000000000000000000281526004018080602001838152602001828103"
                "825260078152602001807f6974656d5f69640000000000000000000000000000000000000000000000"
                "000081525060200192505050600060405180830381600087803b151561089d57fe5b6102c65a03f115"
                "156108ab57fe5b5050508373ffffffffffffffffffffffffffffffffffffffff1663bf2b70a18a8585"
                "6000604051602001526040518463ffffffff167c010000000000000000000000000000000000000000"
                "000000000000000002815260040180806020018473ffffffffffffffffffffffffffffffffffffffff"
                "1673ffffffffffffffffffffffffffffffffffffffff1681526020018373ffffffffffffffffffffff"
                "ffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200182810382"
                "52858181518152602001915080519060200190808383600083146109b4575b80518252602083111561"
                "09b457602082019150602081019050602083039250610990565b505050905090810190601f16801561"
                "09e05780820380516001836020036101000a031916815260200191505b509450505050506020604051"
                "80830381600087803b15156109fd57fe5b6102c65a03f11515610a0b57fe5b50505060405180519050"
                "90507f0bdcb3b747cf033ae78b4b6e1576d2725709d03f68ad3d641b12cb72de614354816040518082"
                "815260200191505060405180910390a18095505b50505050509392505050565b600060006000600060"
                "0061100193508373ffffffffffffffffffffffffffffffffffffffff1663c184e0ff60006040516020"
                "01526040518163ffffffff167c01000000000000000000000000000000000000000000000000000000"
                "000281526004018080602001828103825260068152602001807f745f74657374000000000000000000"
                "0000000000000000000000000000000000815250602001915050602060405180830381600087803b15"
                "15610b1257fe5b6102c65a03f11515610b2057fe5b5050506040518051905092508273ffffffffffff"
                "ffffffffffffffffffffffffffff16637857d7c96000604051602001526040518163ffffffff167c01"
                "0000000000000000000000000000000000000000000000000000000002815260040180905060206040"
                "5180830381600087803b1515610b9857fe5b6102c65a03f11515610ba657fe5b505050604051805190"
                "5091508173ffffffffffffffffffffffffffffffffffffffff1663cd30a1d1886040518263ffffffff"
                "167c010000000000000000000000000000000000000000000000000000000002815260040180806020"
                "0180602001838103835260048152602001807f6e616d65000000000000000000000000000000000000"
                "0000000000000000000081525060200183810382528481815181526020019150805190602001908083"
                "8360008314610c81575b805182526020831115610c8157602082019150602081019050602083039250"
                "610c5d565b505050905090810190601f168015610cad5780820380516001836020036101000a031916"
                "815260200191505b509350505050600060405180830381600087803b1515610cc957fe5b6102c65a03"
                "f11515610cd757fe5b5050508173ffffffffffffffffffffffffffffffffffffffff1663e44594b987"
                "6040518263ffffffff167c010000000000000000000000000000000000000000000000000000000002"
                "81526004018080602001838152602001828103825260078152602001807f6974656d5f696400000000"
                "0000000000000000000000000000000000000000008152506020019250505060006040518083038160"
                "0087803b1515610d7e57fe5b6102c65a03f11515610d8c57fe5b5050508273ffffffffffffffffffff"
                "ffffffffffffffffffff166328bb211788846000604051602001526040518363ffffffff167c010000"
                "000000000000000000000000000000000000000000000000000002815260040180806020018373ffff"
                "ffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16"
                "8152602001828103825284818151815260200191508051906020019080838360008314610e62575b80"
                "5182526020831115610e6257602082019150602081019050602083039250610e3e565b505050905090"
                "810190601f168015610e8e5780820380516001836020036101000a031916815260200191505b509350"
                "505050602060405180830381600087803b1515610eaa57fe5b6102c65a03f11515610eb857fe5b5050"
                "506040518051905090507f896358cb98e9e8e891ae04efd1bc177efbe5cffd7eca2e784b16ed746855"
                "3e08816040518082815260200191505060405180910390a18094505b5050505092915050565b600060"
                "0060006000600061100193508373ffffffffffffffffffffffffffffffffffffffff1663c184e0ff60"
                "00604051602001526040518163ffffffff167c01000000000000000000000000000000000000000000"
                "000000000000000281526004018080602001828103825260068152602001807f745f74657374000000"
                "0000000000000000000000000000000000000000000000815250602001915050602060405180830381"
                "600087803b1515610fbd57fe5b6102c65a03f11515610fcb57fe5b5050506040518051905092508273"
                "ffffffffffffffffffffffffffffffffffffffff166313db93466000604051602001526040518163ff"
                "ffffff167c010000000000000000000000000000000000000000000000000000000002815260040180"
                "9050602060405180830381600087803b151561104357fe5b6102c65a03f1151561105157fe5b505050"
                "6040518051905091508173ffffffffffffffffffffffffffffffffffffffff1663e942b51689604051"
                "8263ffffffff167c010000000000000000000000000000000000000000000000000000000002815260"
                "0401808060200180602001838103835260048152602001807f6e616d65000000000000000000000000"
                "0000000000000000000000000000000081525060200183810382528481815181526020019150805190"
                "602001908083836000831461112c575b80518252602083111561112c57602082019150602081019050"
                "602083039250611108565b505050905090810190601f16801561115857808203805160018360200361"
                "01000a031916815260200191505b509350505050600060405180830381600087803b151561117457fe"
                "5b6102c65a03f1151561118257fe5b5050508173ffffffffffffffffffffffffffffffffffffffff16"
                "632ef8ba74886040518263ffffffff167c010000000000000000000000000000000000000000000000"
                "00000000000281526004018080602001838152602001828103825260078152602001807f6974656d5f"
                "6964000000000000000000000000000000000000000000000000008152506020019250505060006040"
                "5180830381600087803b151561122957fe5b6102c65a03f1151561123757fe5b5050508173ffffffff"
                "ffffffffffffffffffffffffffffffff1663e942b516876040518263ffffffff167c01000000000000"
                "0000000000000000000000000000000000000000000002815260040180806020018060200183810383"
                "5260098152602001807f6974656d5f6e616d6500000000000000000000000000000000000000000000"
                "0081525060200183810382528481815181526020019150805190602001908083836000831461130957"
                "5b805182526020831115611309576020820191506020810190506020830392506112e5565b50505090"
                "5090810190601f1680156113355780820380516001836020036101000a031916815260200191505b50"
                "9350505050600060405180830381600087803b151561135157fe5b6102c65a03f1151561135f57fe5b"
                "5050508273ffffffffffffffffffffffffffffffffffffffff166331afac3689846000604051602001"
                "526040518363ffffffff167c0100000000000000000000000000000000000000000000000000000000"
                "02815260040180806020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffff"
                "ffffffffffffffffffffffffffff168152602001828103825284818151815260200191508051906020"
                "019080838360008314611435575b805182526020831115611435576020820191506020810190506020"
                "83039250611411565b505050905090810190601f168015611461578082038051600183602003610100"
                "0a031916815260200191505b509350505050602060405180830381600087803b151561147d57fe5b61"
                "02c65a03f1151561148b57fe5b5050506040518051905090507f66f7705280112a4d1145399e0414ad"
                "c43a2d6974b487710f417edcf7d4a39d71816040518082815260200191505060405180910390a18094"
                "505b505050509392505050565b600061100190508073ffffffffffffffffffffffffffffffffffffff"
                "ff166356004b6a6000604051602001526040518163ffffffff167c0100000000000000000000000000"
                "0000000000000000000000000000000281526004018080602001806020018060200184810384526006"
                "8152602001807f745f7465737400000000000000000000000000000000000000000000000000008152"
                "50602001848103835260048152602001807f6e616d6500000000000000000000000000000000000000"
                "000000000000000000815250602001848103825260118152602001807f6974656d5f69642c6974656d"
                "5f6e616d65000000000000000000000000000000815250602001935050505060206040518083038160"
                "0087803b15156115fb57fe5b6102c65a03f1151561160957fe5b50505060405180519050505b50565b"
                "611620611f41565b611628611f55565b611630611f41565b6000600060006000611640611f41565b61"
                "1648611f55565b611650611f41565b6000600061100198508873ffffffffffffffffffffffffffffff"
                "ffffffffff1663c184e0ff6000604051602001526040518163ffffffff167c01000000000000000000"
                "0000000000000000000000000000000000000002815260040180806020018281038252600681526020"
                "01807f745f746573740000000000000000000000000000000000000000000000000000815250602001"
                "915050602060405180830381600087803b15156116fe57fe5b6102c65a03f1151561170c57fe5b5050"
                "506040518051905097508773ffffffffffffffffffffffffffffffffffffffff16637857d7c9600060"
                "4051602001526040518163ffffffff167c010000000000000000000000000000000000000000000000"
                "0000000000028152600401809050602060405180830381600087803b151561178457fe5b6102c65a03"
                "f1151561179257fe5b5050506040518051905096508773ffffffffffffffffffffffffffffffffffff"
                "ffff1663e8434e398e896000604051602001526040518363ffffffff167c0100000000000000000000"
                "00000000000000000000000000000000000002815260040180806020018373ffffffffffffffffffff"
                "ffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001828103"
                "825284818151815260200191508051906020019080838360008314611871575b805182526020831115"
                "6118715760208201915060208101905060208303925061184d565b505050905090810190601f168015"
                "61189d5780820380516001836020036101000a031916815260200191505b5093505050506020604051"
                "80830381600087803b15156118b957fe5b6102c65a03f115156118c757fe5b50505060405180519050"
                "95508573ffffffffffffffffffffffffffffffffffffffff1663949d225d6000604051602001526040"
                "518163ffffffff167c0100000000000000000000000000000000000000000000000000000000028152"
                "600401809050602060405180830381600087803b151561193f57fe5b6102c65a03f1151561194d57fe"
                "5b505050604051805190506040518059106119645750595b908082528060200260200182016040525b"
                "5094508573ffffffffffffffffffffffffffffffffffffffff1663949d225d60006040516020015260"
                "40518163ffffffff167c01000000000000000000000000000000000000000000000000000000000281"
                "52600401809050602060405180830381600087803b15156119e457fe5b6102c65a03f115156119f257"
                "fe5b50505060405180519050604051805910611a095750595b90808252806020026020018201604052"
                "5b5093508573ffffffffffffffffffffffffffffffffffffffff1663949d225d600060405160200152"
                "6040518163ffffffff167c010000000000000000000000000000000000000000000000000000000002"
                "8152600401809050602060405180830381600087803b1515611a8957fe5b6102c65a03f11515611a97"
                "57fe5b50505060405180519050604051805910611aae5750595b908082528060200260200182016040"
                "525b509250600091505b8573ffffffffffffffffffffffffffffffffffffffff1663949d225d600060"
                "4051602001526040518163ffffffff167c010000000000000000000000000000000000000000000000"
                "0000000000028152600401809050602060405180830381600087803b1515611b3357fe5b6102c65a03"
                "f11515611b4157fe5b50505060405180519050821215611f27578573ffffffffffffffffffffffffff"
                "ffffffffffffff1663846719e0836000604051602001526040518263ffffffff167c01000000000000"
                "0000000000000000000000000000000000000000000002815260040180828152602001915050602060"
                "405180830381600087803b1515611bc657fe5b6102c65a03f11515611bd457fe5b5050506040518051"
                "905090508073ffffffffffffffffffffffffffffffffffffffff166327314f79600060405160200152"
                "6040518163ffffffff167c010000000000000000000000000000000000000000000000000000000002"
                "81526004018080602001828103825260048152602001807f6e616d6500000000000000000000000000"
                "000000000000000000000000000000815250602001915050602060405180830381600087803b151561"
                "1c8557fe5b6102c65a03f11515611c9357fe5b505050604051805190508583815181101515611cab57"
                "fe5b9060200190602002019060001916908160001916815250508073ffffffffffffffffffffffffff"
                "ffffffffffffff1663fda69fae6000604051602001526040518163ffffffff167c0100000000000000"
                "0000000000000000000000000000000000000000000281526004018080602001828103825260078152"
                "602001807f6974656d5f69640000000000000000000000000000000000000000000000000081525060"
                "2001915050602060405180830381600087803b1515611d6857fe5b6102c65a03f11515611d7657fe5b"
                "505050604051805190508483815181101515611d8e57fe5b90602001906020020181815250508073ff"
                "ffffffffffffffffffffffffffffffffffffff166327314f796000604051602001526040518163ffff"
                "ffff167c01000000000000000000000000000000000000000000000000000000000281526004018080"
                "602001828103825260098152602001807f6974656d5f6e616d65000000000000000000000000000000"
                "0000000000000000815250602001915050602060405180830381600087803b1515611e4157fe5b6102"
                "c65a03f11515611e4f57fe5b505050604051805190508383815181101515611e6757fe5b9060200190"
                "602002019060001916908160001916815250507fc65cd2adf133adee2ddcfab8b165c2f1f7b185c438"
                "9b0789a11112483efb1c848583815181101515611eae57fe5b90602001906020020151858481518110"
                "1515611ec657fe5b906020019060200201518585815181101515611ede57fe5b906020019060200201"
                "5160405180846000191660001916815260200183815260200182600019166000191681526020019350"
                "50505060405180910390a15b816001019150611ac7565b8484849b509b509b505b5050505050505050"
                "509193909250565b602060405190810160405280600081525090565b60206040519081016040528060"
                "00815250905600a165627a7a7230582093b9498b96d50a5b320f578d8d5429fa5f6671fb7f396c5205"
                "d9f6fe2cb8c8e500291ca02d225a81618d4b4720fd26bdbe30925de26be949c33ff84cfeb3bf358007"
                "e2efa0785feb21b69a75402a4feb85e36b970c4d933fbc9dd9f71372eb67e3849387c5");
            dev::bytes rlpBytesCall = dev::fromHex(
                "0xf9016da0010061fb64122b6c683383ef45211e526e72d0412059ee52a5fbcce220e6a40b8401c9c3"
                "808401c9c3808201f494919868496524eedc26dbb81915fa1547a20f899880b8e4ebf3b24f00000000"
                "0000000000000000000000000000000000000000000000000000006000000000000000000000000000"
                "0000000000000000000000000000000000000100000000000000000000000000000000000000000000"
                "000000000000000000a000000000000000000000000000000000000000000000000000000000000000"
                "0161000000000000000000000000000000000000000000000000000000000000000000000000000000"
                "0000000000000000000000000000000000000000000000016300000000000000000000000000000000"
                "0000000000000000000000000000001ba08b0e2cd9c48032ecf68cca6eb951a8b09195d23950555a34"
                "6f6cb9f5f91ea00ea02726ce5352276dbb7bc166dfe036a0ce67ea25848823284c845bf3cf5c6969c"
                "f");
            dev::eth::Transaction tx(ref(rlpBytes), dev::eth::CheckTransaction::Everything);
            // dev::KeyPair key_pair(dev::Secret::random());
            // dev::Secret sec = key_pair.secret();
            // u256 maxBlockLimit = u256(1000);
            // tx.setNonce(tx.nonce() + u256(1));
            // tx.setBlockLimit(u256(blockChain->number()) + maxBlockLimit);
            // dev::Signature sig = sign(sec, tx.sha3(dev::eth::WithoutSignature));
            // tx.updateSignature(SignatureStruct(sig));
            LOG(INFO) << "Tx " << tx;

            dev::eth::Transaction tx2(ref(rlpBytesCall), dev::eth::CheckTransaction::Everything);
            block.appendTransaction(tx);

            block.appendTransaction(tx2);
            LOG(INFO) << "Tx2 " << tx2;
            dev::blockverifier::BlockInfo parentBlockInfo = {parentBlock->header().hash(),
                parentBlock->header().number(), parentBlock->header().stateRoot()};
            auto context = blockVerifier->executeBlock(block, parentBlockInfo);
            blockChain->commitBlock(block, context);
            dev::eth::TransactionReceipt receipt =
                blockChain->getTransactionReceiptByHash(tx.sha3());
            LOG(INFO) << "receipt " << receipt;
            receipt = blockChain->getTransactionReceiptByHash(tx2.sha3());
            LOG(INFO) << "receipt2 " << receipt;
        }
    }
    else if (argc > 1 && std::string("verify") == argv[1])
    {
    }
}