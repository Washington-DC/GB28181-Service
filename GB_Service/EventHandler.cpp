#include "pch.h"
#include "EventHandler.h"
#include "ConfigManager.h"
#include "HttpDigest.h"
#include "ConfigManager.h"
#include "DeviceManager.h"

int BaseEventHandler::SendResponse(const char* uname, eXosip_t* excontext, int tid, int status)
{
	osip_message_t* answer = nullptr;

	eXosip_lock(excontext);
	eXosip_message_build_answer(excontext, tid, status, &answer);
	int ret = eXosip_message_send_answer(excontext, tid, status, nullptr);
	eXosip_unlock(excontext);
	return ret;
}

int BaseEventHandler::SendCallAck(eXosip_t* excontext, int did)
{
	eXosip_lock(excontext);
	osip_message_t* ack = nullptr;
	eXosip_call_build_ack(excontext, did, &ack);
	int ret = eXosip_call_send_ack(excontext, did, ack);
	eXosip_unlock(excontext);
	return ret;
}

int BaseEventHandler::GetStatusCodeFromResponse(osip_message_t* response)
{
	return response != nullptr ? response->status_code : -1;
}

std::string BaseEventHandler::GetMsgIDFromRequest(osip_message_t* request)
{
	osip_generic_param_t* tag = nullptr;
	osip_to_get_tag(request->from, &tag);
	if (nullptr == tag || nullptr == tag->gvalue) {
		return "";
	}
	return tag->gvalue;
}

int RegisterHandler::HandleIncomingRequest(const SipEvent::Ptr& e)
{
	osip_authorization_t* authorization = nullptr;
	osip_message_get_authorization(e->exosip_event->request, 0, &authorization);

	if (authorization && authorization->username)
	{
		char* method = nullptr;
		//char* algorithm = nullptr;
		char* username = nullptr;
		//char* realm = nullptr;
		//char* nonce = nullptr;
		//char* nonce_count = nullptr;
		char* uri = nullptr;
		char* response = nullptr;

		osip_contact_t* contact = nullptr;
		osip_message_get_contact(e->exosip_event->request, 0, &contact);
		method = e->exosip_event->request->sip_method;

#define SIP_STRDUP(field)  if(authorization->field) (field) = osip_strdup_without_quote(authorization->field);

		//SIP_STRDUP(algorithm);
		SIP_STRDUP(username);
		//SIP_STRDUP(realm);
		//SIP_STRDUP(nonce);
		//SIP_STRDUP(nonce_count);
		SIP_STRDUP(uri);
		SIP_STRDUP(response);

		auto config = ConfigManager::GetInstance()->GetSipServerInfo();

		//DigestCalcHA1(algorithm, username, realm, config->Password.c_str(), nonce, nonce_count, HA1);
		//DigestCalcResponse(HA1, nonce, nonce_count, authorization->cnonce, authorization->message_qop,
		//	0, method, uri, HA2, Response);

		HASHHEX HA1, calc_response;
		DigestCalcHA1("REGISTER", username, config->Realm.c_str(), config->Password.c_str(),
			config->Nonce.c_str(), NULL, HA1);
		DigestCalcResponse(HA1, config->Nonce.c_str(), NULL, NULL, NULL, 0, method, uri, NULL,
			calc_response);


		LOG(INFO) << "MD5: " << calc_response;
		//memcpy(calc_response, temp_response, HASHHEXLEN);

		std::string client_host = strdup(contact->url->host);
		auto client_port = strdup(contact->url->port);
		auto client_device_id = username;

		if (0 == memcmp(calc_response, response, HASHHEXLEN))
		{
			SendResponse(username, e->exosip_context, e->exosip_event->tid, SIP_OK);
			LOG(INFO) << "Device Registration Success, Address:" << client_host << ":" << client_port << "   ID: " << client_device_id;

			//TOOD:
			auto device = std::make_shared<Device>(client_device_id, client_host, client_port);
			device->SetStatus(1);
			//device
			DeviceManager::GetInstance()->AddDevice(device);
		}
		else
		{
			SendResponse(username, e->exosip_context, e->exosip_event->tid, SIP_UNAUTHORIZED);
			LOG(INFO) << "Device Registration Failed, Address:" << client_host << ":" << client_port << "   ID: " << client_device_id;

			//TODO:
			DeviceManager::GetInstance()->RemoveDevice(client_device_id);
		}
		osip_free(uri);
		osip_free(username);
		osip_free(response);
	}
	else
	{
		_response_register_401unauthorized(e);
	}

	return 0;
}

void RegisterHandler::_response_register_401unauthorized(const SipEvent::Ptr& e)
{
	osip_www_authenticate_t* www_authenticate_header = nullptr;
	osip_www_authenticate_init(&www_authenticate_header);

	//std::string realm = "cdtye";
	char* dest = nullptr;
	osip_message_t* response = nullptr;

	auto config = ConfigManager::GetInstance()->GetSipServerInfo();

	osip_www_authenticate_set_auth_type(www_authenticate_header, osip_strdup("Digest"));
	osip_www_authenticate_set_realm(www_authenticate_header, osip_enquote(config->Realm.c_str()));
	osip_www_authenticate_set_nonce(www_authenticate_header, osip_enquote(config->Nonce.c_str()));
	osip_www_authenticate_to_str(www_authenticate_header, &dest);
	int ret = eXosip_message_build_answer(e->exosip_context, e->exosip_event->tid, SIP_UNAUTHORIZED, &response);
	if (ret == 0 && response != nullptr)
	{
		osip_message_set_www_authenticate(response, dest);
		osip_message_set_content_type(response, "Application/MANSCDP+xml");
		eXosip_lock(e->exosip_context);
		eXosip_message_send_answer(e->exosip_context, e->exosip_event->tid, SIP_UNAUTHORIZED, response);
		eXosip_unlock(e->exosip_context);
		LOG(INFO) << "response_register_401unauthorized success";
	}
	else
	{
		LOG(ERROR) << "response_register_401unauthorized error";
	}

	osip_www_authenticate_free(www_authenticate_header);
	osip_free(dest);
}
