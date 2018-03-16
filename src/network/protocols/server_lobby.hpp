#ifndef SERVER_LOBBY_HPP
#define SERVER_LOBBY_HPP

#include "network/protocols/lobby_protocol.hpp"
#include "utils/cpp2011.hpp"

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <tuple>

class STKPeer;

class ServerLobby : public LobbyProtocol
{
public:
    /* The state for a small finite state machine. */
    enum ServerState : unsigned int
    {
        SET_PUBLIC_ADDRESS,       // Waiting to receive its public ip address
        REGISTER_SELF_ADDRESS,    // Register with STK online server
        ACCEPTING_CLIENTS,        // In lobby, accepting clients
        SELECTING,                // kart, track, ... selection started
        LOAD_WORLD,               // Server starts loading world
        WAIT_FOR_WORLD_LOADED,    // Wait for clients and server to load world
        WAIT_FOR_RACE_STARTED,    // Wait for all clients to have started the race
        DELAY_SERVER,             // Additional server delay
        RACING,                   // racing
        RESULT_DISPLAY,           // Show result screen
        ERROR_LEAVE,              // shutting down server
        EXITING
    };
private:
    std::atomic<ServerState> m_state;

    /** Hold the next connected peer for server owner if current one expired
     * (disconnected). */
    std::weak_ptr<STKPeer> m_server_owner;

    /** Available karts and tracks for all clients, this will be initialized
     *  with data in server first. */
    std::pair<std::set<std::string>, std::set<std::string> > m_available_kts;

    /** Keeps track of the server state. */
    std::atomic_bool m_server_has_loaded_world;

    /** Counts how many peers have finished loading the world. */
    std::map<std::weak_ptr<STKPeer>, bool,
        std::owner_less<std::weak_ptr<STKPeer> > > m_peers_ready;

    /** Vote from each peer. */
    std::map<std::weak_ptr<STKPeer>, std::tuple<std::string, uint8_t, bool>,
        std::owner_less<std::weak_ptr<STKPeer> > > m_peers_votes;

    /** Keeps track of an artificial server delay (which makes sure that the
     *  data from all clients has arrived when the server computes a certain
     *  timestep.(. It stores the real time since epoch + delta (atm 0.1
     *  seconds), which is the real time at which the server should start. */
    double m_server_delay;

    bool m_has_created_server_id_file;

    /** It indicates if this server is registered with the stk server. */
    std::atomic_bool m_server_registered;

    /** Counts how many players are ready to go on. */
    int m_player_ready_counter;

    /** Timeout counter for various state. */
    float m_timeout;

    /** Lock this mutex whenever a client is connect / disconnect or
     *  starting race. */
    std::mutex m_connection_mutex;

    // connection management
    void clientDisconnected(Event* event);
    void connectionRequested(Event* event);
    // kart selection
    void kartSelectionRequested(Event* event);
    // Track(s) votes
    void playerVote(Event *event);
    void playerFinishedResult(Event *event);
    void registerServer();
    void finishedLoadingWorldClient(Event *event);
    void startedRaceOnClient(Event *event);
    void kickHost(Event* event);
    void handleChat(Event* event);
    void unregisterServer();
    void createServerIdFile();
    void updatePlayerList();
    void updateServerOwner();
    bool checkPeersReady() const;
    void handleVote();

public:
             ServerLobby();
    virtual ~ServerLobby();

    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE;
    virtual bool notifyEvent(Event* event) OVERRIDE;
    virtual void setup() OVERRIDE;
    virtual void update(float dt) OVERRIDE;
    virtual void asynchronousUpdate() OVERRIDE;

    void signalRaceStartToClients();
    void startSelection(const Event *event=NULL);
    void checkIncomingConnectionRequests();
    void checkRaceFinished();
    void finishedLoadingWorld() OVERRIDE;
    ServerState getCurrentState() const { return m_state.load(); }
    virtual bool waitingForPlayers() const OVERRIDE
                                { return m_state.load() == ACCEPTING_CLIENTS; }

};   // class ServerLobby

#endif // SERVER_LOBBY_HPP
