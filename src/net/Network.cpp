#ifdef _MSC_VER
#pragma warning(disable:4244)
#endif

#include <inttypes.h>
#include <memory>
#include <time.h>


#include "api/Api.h"
#include "log/Log.h"
#include "net/Client.h"
#include "net/Network.h"
#include "net/strategies/DonateStrategy.h"
#include "net/strategies/FailoverStrategy.h"
#include "net/strategies/SinglePoolStrategy.h"
#include "net/SubmitResult.h"
#include "net/Url.h"
#include "Options.h"
#include "Platform.h"
#include "workers/Workers.h"


Network::Network(const Options *options) :
    m_options(options),
    m_donate(nullptr)
{
    srand(time(0) ^ (uintptr_t) this);

    Workers::setListener(this);

    const std::vector<Url*> &pools = options->pools();

    if (pools.size() > 1) {
        m_strategy = new FailoverStrategy(pools, Platform::userAgent(), this);
    }
    else {
        m_strategy = new SinglePoolStrategy(pools.front(), Platform::userAgent(), this);
    }

    if (m_options->donateLevel() > 0) {
        m_donate = new DonateStrategy(Platform::userAgent(), this);
    }

    m_timer.data = this;
    uv_timer_init(uv_default_loop(), &m_timer);

    uv_timer_start(&m_timer, Network::onTick, kTickInterval, kTickInterval);
}


Network::~Network()
{
}


void Network::connect()
{
    m_strategy->connect();
}


void Network::stop()
{
    if (m_donate) {
        m_donate->stop();
    }

    m_strategy->stop();
}


void Network::onActive(Client *client)
{
    if (client->id() == -1) {
        
        return;
    }

    m_state.setPool(client->host(), client->port(), client->ip());

    
}


void Network::onJob(Client *client, const Job &job)
{
    if (m_donate && m_donate->isActive() && client->id() != -1) {
        return;
    }

    setJob(client, job);
}


void Network::onJobResult(const JobResult &result)
{
    if (result.poolId == -1 && m_donate) {
        m_donate->submit(result);
        return;
    }

    m_strategy->submit(result);
}


void Network::onPause(IStrategy *strategy)
{
    if (m_donate && m_donate == strategy) {
        
        m_strategy->resume();
    }

    if (!m_strategy->isActive()) {
        
        m_state.stop();
        return Workers::pause();
    }
}


void Network::onResultAccepted(Client *client, const SubmitResult &result, const char *error)
{
    m_state.add(result, error);

    if (error) {
        
    }
    else {
      
    }
}


void Network::setJob(Client *client, const Job &job)
{
    if (m_options->colors()) {
      
    }
    else {
        
    }

    m_state.diff = job.diff();
    Workers::setJob(job);
}


void Network::tick()
{
    const uint64_t now = uv_now(uv_default_loop());

    m_strategy->tick(now);

    if (m_donate) {
        m_donate->tick(now);
    }

#   ifndef gchgrp_NO_API
    Api::tick(m_state);
#   endif
}


void Network::onTick(uv_timer_t *handle)
{
    static_cast<Network*>(handle->data)->tick();
}
