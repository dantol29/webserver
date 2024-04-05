#include "HTTPRequest.hpp"

bool isOrForm(std::string requestTarget, int &queryStart){
	for (int i = 0; i < (int)requestTarget.length(); i++){
		if (requestTarget[i] == '?'){
			queryStart = i;
			return (true);
		}
	}
	return (false);
}

bool fileExists(std::string requestTarget, bool isOriginForm, int queryStart){
	if (isOriginForm && \
	access(("." + requestTarget.substr(0, queryStart)).c_str(), F_OK) == -1)
		return (false);
	if (!isOriginForm && access(("." + requestTarget).c_str(), F_OK) == -1)
		return (false);
	return (true);
}
