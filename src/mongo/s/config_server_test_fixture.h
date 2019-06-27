/**
 *    Copyright (C) 2018-present MongoDB, Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the Server Side Public License, version 1,
 *    as published by MongoDB, Inc.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    Server Side Public License for more details.
 *
 *    You should have received a copy of the Server Side Public License
 *    along with this program. If not, see
 *    <http://www.mongodb.com/licensing/server-side-public-license>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the Server Side Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */

#pragma once

#include "mongo/s/sharding_mongod_test_fixture.h"

namespace mongo {

class BSONObj;
class ChunkType;
struct ChunkVersion;
class KeysCollectionDocument;
class NamespaceString;
class Shard;
class ShardId;
class ShardRegistry;
class ShardType;
template <typename T>
class StatusWith;

/**
 * Provides config-specific functionality in addition to the mock storage engine and mock network
 * provided by ShardingMongodTestFixture.
 */
class ConfigServerTestFixture : public ShardingMongodTestFixture {
public:
    ConfigServerTestFixture();
    ~ConfigServerTestFixture();

    std::shared_ptr<Shard> getConfigShard() const;

    /**
     * Insert a document to this config server to the specified namespace.
     */
    Status insertToConfigCollection(OperationContext* opCtx,
                                    const NamespaceString& ns,
                                    const BSONObj& doc);

    /**
     * Updates a document to this config server to the specified namespace.
     */
    Status updateToConfigCollection(OperationContext* opCtx,
                                    const NamespaceString& ns,
                                    const BSONObj& query,
                                    const BSONObj& update,
                                    const bool upsert);

    /**
     * Deletes a document to this config server to the specified namespace.
     */
    Status deleteToConfigCollection(OperationContext* opCtx,
                                    const NamespaceString& ns,
                                    const BSONObj& doc,
                                    const bool multi);

    /**
     * Reads a single document from a collection living on the config server.
     */
    StatusWith<BSONObj> findOneOnConfigCollection(OperationContext* opCtx,
                                                  const NamespaceString& ns,
                                                  const BSONObj& filter);

    /**
     * Setup the config.shards collection to contain the given shards.
     */
    void setupShards(const std::vector<ShardType>& shards);

    /**
     * Retrieves the shard document from the config server.
     * Returns {ErrorCodes::ShardNotFound} if the given shard does not exists.
     */
    StatusWith<ShardType> getShardDoc(OperationContext* opCtx, const std::string& shardId);

    /**
     * Setup the config.chunks collection to contain the given chunks.
     */
    void setupChunks(const std::vector<ChunkType>& chunks);

    /**
     * Retrieves the chunk document from the config server.
     */
    StatusWith<ChunkType> getChunkDoc(OperationContext* opCtx, const BSONObj& minKey);

    /**
     * Inserts a document for the database into the config.databases collection.
     */
    void setupDatabase(const std::string& dbName, const ShardId primaryShard, const bool sharded);

    /**
     * Returns the indexes definitions defined on a given collection.
     */
    StatusWith<std::vector<BSONObj>> getIndexes(OperationContext* opCtx, const NamespaceString& ns);

    /**
     * Expects a setShardVersion command to be executed on the specified shard.
     *
     * The expectedChunkVersion is optional, because in some cases it may not be possible to know
     * the OID of a ChunkVersion generated by some internal code. (See SERVER-29451).
     */
    void expectSetShardVersion(const HostAndPort& expectedHost,
                               const ShardType& expectedShard,
                               const NamespaceString& expectedNs,
                               boost::optional<ChunkVersion> expectedChunkVersion);

    /**
     * Returns the stored raw pointer to the addShard TaskExecutor's NetworkInterface.
     */
    executor::NetworkInterfaceMock* networkForAddShard() const;

    /**
     * Returns the stored raw pointer to the addShard TaskExecutor.
     */
    executor::TaskExecutor* executorForAddShard() const;

    /**
     * Same as ShardingMongodTestFixture::onCommand but run against _addShardNetworkTestEnv.
     */
    void onCommandForAddShard(executor::NetworkTestEnv::OnCommandFunction func);

    /**
     * Returns all the keys in admin.system.keys
     */
    std::vector<KeysCollectionDocument> getKeys(OperationContext* opCtx);

protected:
    /**
     * Sets this node up as a mongod with sharding components for ClusterRole::ConfigServer.
     */
    void setUp() override;
    void tearDown() override;

    std::unique_ptr<DistLockCatalog> makeDistLockCatalog() override;

    std::unique_ptr<DistLockManager> makeDistLockManager(
        std::unique_ptr<DistLockCatalog> distLockCatalog) override;

    std::unique_ptr<ShardingCatalogClient> makeShardingCatalogClient(
        std::unique_ptr<DistLockManager> distLockManager) override;

    std::unique_ptr<ClusterCursorManager> makeClusterCursorManager() override;

    std::unique_ptr<BalancerConfiguration> makeBalancerConfiguration() override;

private:
    // Since these are currently private members of the real ShardingCatalogManager, we store a raw
    // pointer to them here.
    executor::NetworkInterfaceMock* _mockNetworkForAddShard;
    executor::TaskExecutor* _executorForAddShard;

    // Allows for processing tasks through the NetworkInterfaceMock/ThreadPoolMock subsystem.
    std::unique_ptr<executor::NetworkTestEnv> _addShardNetworkTestEnv;
};

}  // namespace mongo
