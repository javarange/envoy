#include "server/config/network/mongo_proxy.h"

#include <string>

#include "envoy/network/connection.h"
#include "envoy/server/instance.h"

#include "common/json/config_schemas.h"
#include "common/mongo/proxy.h"

namespace Envoy {
namespace Server {
namespace Configuration {

NetworkFilterFactoryCb MongoProxyFilterConfigFactory::createFilterFactory(
    NetworkFilterType type, const Json::Object& config, Server::Instance& server) {
  if (type != NetworkFilterType::Both) {
    throw EnvoyException(fmt::format(
        "{} network filter must be configured as both a read and write filter.", name()));
  }

  config.validateSchema(Json::Schema::MONGO_PROXY_NETWORK_FILTER_SCHEMA);

  std::string stat_prefix = "mongo." + config.getString("stat_prefix") + ".";
  Mongo::AccessLogSharedPtr access_log;
  if (config.hasObject("access_log")) {
    access_log.reset(
        new Mongo::AccessLog(config.getString("access_log"), server.accessLogManager()));
  }

  return [stat_prefix, &server, access_log](Network::FilterManager& filter_manager) -> void {
    filter_manager.addFilter(Network::FilterSharedPtr{
        new Mongo::ProdProxyFilter(stat_prefix, server.stats(), server.runtime(), access_log)});
  };
}

std::string MongoProxyFilterConfigFactory::name() { return "mongo_proxy"; }

/**
 * Static registration for the mongo filter. @see RegisterNamedNetworkFilterConfigFactory.
 */
static RegisterNamedNetworkFilterConfigFactory<MongoProxyFilterConfigFactory> registered_;

} // Configuration
} // Server
} // Envoy
