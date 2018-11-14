/*
 * ZeroTier One - Network Virtualization Everywhere
 * Copyright (C) 2011-2018  ZeroTier, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#ifdef ZT_CONTROLLER_USE_LIBPQ

#ifndef ZT_CONTROLLER_LIBPQ_HPP
#define ZT_CONTROLLER_LIBPQ_HPP

#include "DB.hpp"

#include <pqxx/pqxx>

extern "C" {
    typedef struct pg_conn PGconn;
}

namespace ZeroTier
{
class _MemberNotificationReceiver;
class _NetworkNotificationReceiver;

/**
 * A controller database driver that talks to PostgreSQL
 *
 * This is for use with ZeroTier Central.  Others are free to build and use it
 * but be aware taht we might change it at any time.
 */
class PostgreSQL : public DB
{
public:
    PostgreSQL(EmbeddedNetworkController *const nc, const Identity &myId, const char *path);
    virtual ~PostgreSQL();

    virtual bool waitForReady();
    virtual bool isReady();
    virtual void save(nlohmann::json *orig, nlohmann::json &record);
    virtual void eraseNetwork(const uint64_t networkId);
    virtual void eraseMember(const uint64_t networkId, const uint64_t memberId);
    virtual void nodeIsOnline(const uint64_t networkId, const uint64_t memberId, const InetAddress &physicalAddress);

protected:
    struct _PairHasher
	{
		inline std::size_t operator()(const std::pair<uint64_t,uint64_t> &p) const { return (std::size_t)(p.first ^ p.second); }
	};

private:
    void initializeNetworks(pqxx::connection &conn);
    void initializeMembers(pqxx::connection &conn);
    void heartbeat();
    void membersDbWatcher();
    void networksDbWatcher();
    void commitThread();
    void onlineNotificationThread();

    std::string _connString;

    BlockingQueue<nlohmann::json *> _commitQueue;


    std::thread _heartbeatThread;
    std::thread _membersDbWatcher;
    std::thread _networksDbWatcher;
    std::thread _commitThread[ZT_CONTROLLER_RETHINKDB_COMMIT_THREADS];
    std::thread _onlineNotificationThread;

	std::unordered_map< std::pair<uint64_t,uint64_t>,std::pair<int64_t,InetAddress>,_PairHasher > _lastOnline;

    mutable std::mutex _lastOnline_l;
    mutable std::mutex _readyLock;
    std::atomic<int> _ready, _connected, _run;
    mutable volatile bool _waitNoticePrinted;

    friend class _MemberNotificationReceiver;
    friend class _NetworkNotificationReceiver;
};

}

#endif // ZT_CONTROLLER_LIBPQ_HPP

#endif // ZT_CONTROLLER_USE_LIBPQ