#ifndef INCLUDE_WORLD_NPC_LIVINGNPCPROFILE_HPP
#define INCLUDE_WORLD_NPC_LIVINGNPCPROFILE_HPP

#include <string>

struct LivingNpcProfile
{
    std::string temperament = "prudent";
    std::string profession = "habitant";
    std::string nativeLanguage = "commun";
    std::string informationNetwork = "voisinage";
    bool canInitiateConversation = true;
    bool remembersLocalEvents = true;
};

class LivingNpcProfileSystem
{
public:
    static LivingNpcProfile infer(const std::string& name, const std::string& role, const std::string& raceName);
    static std::string reactionToKnownFact(const LivingNpcProfile& profile, const std::string& factType, const std::string& subjectName);
};

#endif
