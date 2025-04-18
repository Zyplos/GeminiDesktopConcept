#include "geminiAPI.hpp"

using json = nlohmann::json;

// initializer list
GeminiClient::GeminiClient(std::string apiKey) :
    GEMINI_KEY(apiKey),
    state(State::IDLE) {}

void GeminiClient::reset() {
    state = State::IDLE;
    errorFeedback = "";
}

bool GeminiClient::isClientDoingSomething() {
    return state != GeminiClient::IDLE;
}

bool GeminiClient::callAPI(std::string prompt, std::string clipboardText) {
    if (state != State::IDLE) {
        return false;
    }
    state = State::RUNNING;

    std::string promptWithText = prompt + "\n\n" + clipboardText;

    // FAILED quote test "this should fail" end text
    // FAILED slash test \this might fail\
    // semi quote test 'this might fail' end text
    // test ; , " \ , end text

    std::string escapedPrompt;
    for (char c : promptWithText) {
        if (c == '"' || c == '\\') { 
            escapedPrompt += '\\'; 
        }
        escapedPrompt += c;
    }

    cpr::PostCallback(
        // c++ lambda
        [this](cpr::Response response) {
            processResponse(response);
        },

        cpr::Url{ "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" + GEMINI_KEY },
        cpr::Body{ "{ \"contents\": [ { \"role\": \"user\", \"parts\": [ { \"text\": \"" + escapedPrompt + "\" }, ] }, ], \"generationConfig\": { \"responseMimeType\": \"application/json\", \"responseSchema\": { \"type\": \"object\", \"properties\": { \"suggestions\": { \"type\": \"array\", \"items\": { \"type\": \"string\" } } }, \"required\": [ \"suggestions\" ] }, }, \"safetySettings\": [ { \"category\": \"HARM_CATEGORY_CIVIC_INTEGRITY\", \"threshold\": \"BLOCK_LOW_AND_ABOVE\" }, ], }" },
        cpr::Header{ {"Content-Type", "application/json"} }
    );

    return true;
}

void GeminiClient::processResponse(cpr::Response response) {
    //std::cout << "!!!!!!!!!!! RAW RESPONSE" << response.text << std::endl;

    if (response.status_code != 200) {
        state = State::FAILED;
        errorFeedback = "There was an error trying to get suggestions from Gemini. (HTTP " + std::to_string(response.status_code) + ")";

        try {
            json jsonData = json::parse(response.text);
            errorFeedback += "\n" + jsonData["error"]["message"].template get<std::string>();
        }
        catch (json::exception e) {
            // double error
        }

        return;
    }

    try {
        json data = json::parse(response.text);
        std::string suggestionsRaw = data["candidates"][0]["content"]["parts"][0]["text"];

        json suggestionsJson = json::parse(suggestionsRaw);
        suggestions = suggestionsJson["suggestions"].template get<std::vector<std::string>>();
        state = State::FINISHED;
    }
    catch (json::exception e) {
        state = State::FAILED;
        std::string errorString(e.what());
        errorFeedback = "There was an error trying to parse Gemini's reponse.\n\n" + errorString;
    }
}

std::string GeminiClient::getPrompt(PromptType type) {
    switch (type) {
    case PromptType::SYNONYMS:
        return "If the following text snippet is a single word or a two word phrase, give me 10 synonyms for it. Otherwise give me 10 ways to say something similar. If suggestions cannot be expressed in 10 words due to needing more context or other reasons, give a variety of suggestions that span multiple possible contexts.";
        break;
    case PromptType::REPHRASE:
        return "Give me 10 other ways to rephrase the following text snippet:";
        break;
    case PromptType::FORMALIZE:
        return "Make the following text snippet more formal: ";
        break;
    case PromptType::ANTONYMS:
        return "If the following text snippet is a single word or a two word phrase, give me 10 antonyms for it. Otherwise give me 10 ways to say the opposite thing while still keeping the same intent. If suggestions cannot be expressed in 10 words due to needing more context or other reasons, give a variety of suggestions that span multiple possible contexts.";
        break;
    case PromptType::UNGARBLE:
        return "This snippet of text doesn't sound quite right, please rewrite it while keeping the same tone, intent, and meaning.";
        break;
    case PromptType::SHORTEN:
        return "Make the following text snippet shorter:";
        break;
        // -----
    case PromptType::HEADLINE:
        return "Turn the following text snippet into a headline, like, for example, the title for a blog post or web page:";
        break;
    case PromptType::TAGLINE:
        return "Rewrite the following text snipport into a tag line:";
        break;
    case PromptType::ONEWORD:
        return "Rewrite the following text snippet into a one word phrase that encapsulates the same meaning:";
        break;
    case PromptType::TWOWORD:
        return "Rewrite the following text snippet into two word phrase that encapsulates the same meaning:";
        break;
    default:
        return "";
        break;
    }
}

void GeminiClient::debug() {
    state = State::FINISHED;
    suggestions.push_back("Pidgey has an extremely sharp sense of direction. It is capable of unerringly returning home to its nest, however far it may be removed from its familiar surroundings.If Horsea senses danger, it will reflexively spray a dense black ink from its mouth and try to escape. This Pokémon swims by cleverly flapping the fin on its back.One of Electrode’s characteristics is its attraction to electricity. It is a problematical Pokémon that congregates mostly at electrical power plants to feed on electricity that has just been generated.");

    suggestions.push_back("Most of Gulpin’s body is made up of its stomach—its heart and brain are very small in comparison. This Pokémon’s stomach contains special enzymes that dissolve anything.Seviper’s swordlike tail serves two purposes—it slashes foes and douses them with secreted poison. This Pokémon will not give up its long-running blood feud with Zangoose.Illumise leads a flight of illuminated Volbeat to draw signs in the night sky. This Pokémon is said to earn greater respect from its peers by composing more complex designs in the sky.");

    suggestions.push_back("Suicune embodies the compassion of a pure spring of water. It runs across the land with gracefulness. This Pokémon has the power to purify dirty water.In many places, there are folktales of stardust falling into the ocean and becoming Staryu.Its incisors grow continuously throughout its life. If its incisors get too long, this Pokémon becomes unable to eat, and it starves to death.");

    suggestions.push_back("If Shroomish senses danger, it shakes its body and scatters spores from the top of its head. This Pokémon’s spores are so toxic, they make trees and weeds wilt.This Pokémon is nocturnal. Even in total darkness, its large eyes can spot its prey clearly!On moonless nights, Haunter searches for someone to curse, so it’s best not to go out walking around.");

    suggestions.push_back("Through Primal Reversion and with nature’s full power, it will take back its true form. It can cause magma to erupt and expand the landmass of the world.It is vindictive and relentless by nature. Those who cross it even once will be cursed for a thousand years, along with their descendants.It can’t help but hear a pin drop from over half a mile away, so it lives deep in the mountains where there aren’t many people or Pokémon.");

    suggestions.push_back("It fires off ultrasonic waves from its red orbs to weaken its prey, and then it wraps them up in its 80 tentacles.It places small Pokémon into its spacious beak and carries them around. No one knows where it’s taking them.In many places, there are folktales of stardust falling into the ocean and becoming Staryu.");

    suggestions.push_back("Entei embodies the passion of magma. This Pokémon is thought to have been born in the eruption of a volcano. It sends up massive bursts of fire that utterly consume all that they touch.Pidgey has an extremely sharp sense of direction. It is capable of unerringly returning home to its nest, however far it may be removed from its familiar surroundings.With its two brains, it fires powerful psychic energy to stop its prey in their tracks.");

    suggestions.push_back("Bayleef’s neck is ringed by curled-up leaves. Inside each tubular leaf is a small shoot of a tree. The fragrance of this shoot makes people peppy.Krabby live on beaches, burrowed inside holes dug into the sand. On sandy beaches with little in the way of food, these Pokémon can be seen squabbling with each other over territory.Azurill’s tail is large and bouncy. It is packed full of the nutrients this Pokémon needs to grow. Azurill can be seen bouncing and playing on its big, rubbery tail.");

    suggestions.push_back("Whismur is very timid. If it starts to cry loudly, it becomes startled by its own crying and cries even harder. When it finally stops crying, the Pokémon goes to sleep, all tired out.The pink of Corsola that live in Alola is deep and vibrant, thanks to seas filled with nutrition.Weezing alternately shrinks and inflates its twin bodies to mix together toxic gases inside. The more the gases are mixed, the more powerful the toxins become. The Pokémon also becomes more putrid.");

    suggestions.push_back("Kyogre is said to be the personification of the sea itself. Legends tell of its many clashes against Groudon, as each sought to gain the power of nature.Zapdos is a legendary bird Pokémon that has the ability to control electricity. It usually lives in thunderclouds. The Pokémon gains power if it is stricken by lightning bolts.They’re popular, but they’re rare. Trainers who show them off recklessly may be targeted by thieves.");

    suggestions.push_back("Swellow is very conscientious about the upkeep of its glossy wings. Once two Swellow are gathered, they diligently take care of cleaning each other’s wings.Whenever it sees bird Pokémon flying through the sky, it becomes envious and smashes its surroundings to bits with headbutts.It lives in areas of limited rainfall. When danger approaches, it curls up into a ball to protect its soft stomach.");

    suggestions.push_back("Azurill’s tail is large and bouncy. It is packed full of the nutrients this Pokémon needs to grow. Azurill can be seen bouncing and playing on its big, rubbery tail.You can hear tales told all over the world about how Gengar will pay a visit to children who are naughty.Bulbasaur can be seen napping in bright sunlight. There is a seed on its back. By soaking up the sun’s rays, the seed grows progressively larger.");

    suggestions.push_back("Every once in a while, you’ll see a Golbat that’s missing some fangs. This happens when hunger drives it to try biting a Steel-type Pokémon.People believe that carrying one of its discarded fangs will prevent mishaps at sea, so the fangs are made into accessories.Charmeleon mercilessly destroys its foes using its sharp claws. If it encounters a strong foe, it turns aggressive. In this excited state, the flame at the tip of its tail flares with a bluish white color.");

    suggestions.push_back("Azurill’s tail is large and bouncy. It is packed full of the nutrients this Pokémon needs to grow. Azurill can be seen bouncing and playing on its big, rubbery tail.When it encounters an enemy that’s truly mighty, this Pokémon removes the power-save belt from its waist and unleashes its full power.Its body is like steel. Its tough, heavy pincers are more suited to smashing enemies than grabbing them.");

    suggestions.push_back("The fragrance of Meganium’s flower soothes and calms emotions. In battle, this Pokémon gives off more of its becalming scent to blunt the foe’s fighting spirit.People believe that carrying one of its discarded fangs will prevent mishaps at sea, so the fangs are made into accessories.Their food sources have decreased, and their numbers have declined sharply. Sludge ponds are being built to prevent their extinction.");

    suggestions.push_back("Swampert predicts storms by sensing subtle differences in the sounds of waves and tidal winds with its fins. If a storm is approaching, it piles up boulders to protect itself.Doduo’s two heads contain completely identical brains. A scientific study reported that on rare occasions, there will be examples of this Pokémon possessing different sets of brains.Overjoyed at finally being able to fly, it flies all over the place and usually doesn’t land until it’s completely exhausted and needs to sleep.");

    suggestions.push_back("When something approaches it, it fires off fragments of its steel shell in attack. This is not a conscious action but a conditioned reflex.As electricity builds up inside its body, it becomes more aggressive. One theory is that the electricity buildup is actually causing stress.Togetic is said to be a Pokémon that brings good fortune. When the Pokémon spots someone who is pure of heart, it is said to appear and share its happiness with that person.");

    suggestions.push_back("Pidgey has an extremely sharp sense of direction. It is capable of unerringly returning home to its nest, however far it may be removed from its familiar surroundings.If Horsea senses danger, it will reflexively spray a dense black ink from its mouth and try to escape. This Pokémon swims by cleverly flapping the fin on its back.One of Electrode’s characteristics is its attraction to electricity. It is a problematical Pokémon that congregates mostly at electrical power plants to feed on electricity that has just been generated.");

    suggestions.push_back("Most of Gulpin’s body is made up of its stomach—its heart and brain are very small in comparison. This Pokémon’s stomach contains special enzymes that dissolve anything.Seviper’s swordlike tail serves two purposes—it slashes foes and douses them with secreted poison. This Pokémon will not give up its long-running blood feud with Zangoose.Illumise leads a flight of illuminated Volbeat to draw signs in the night sky. This Pokémon is said to earn greater respect from its peers by composing more complex designs in the sky.");

    suggestions.push_back("Suicune embodies the compassion of a pure spring of water. It runs across the land with gracefulness. This Pokémon has the power to purify dirty water.In many places, there are folktales of stardust falling into the ocean and becoming Staryu.Its incisors grow continuously throughout its life. If its incisors get too long, this Pokémon becomes unable to eat, and it starves to death.");

    suggestions.push_back("If Shroomish senses danger, it shakes its body and scatters spores from the top of its head. This Pokémon’s spores are so toxic, they make trees and weeds wilt.This Pokémon is nocturnal. Even in total darkness, its large eyes can spot its prey clearly!On moonless nights, Haunter searches for someone to curse, so it’s best not to go out walking around.");

    suggestions.push_back("Through Primal Reversion and with nature’s full power, it will take back its true form. It can cause magma to erupt and expand the landmass of the world.It is vindictive and relentless by nature. Those who cross it even once will be cursed for a thousand years, along with their descendants.It can’t help but hear a pin drop from over half a mile away, so it lives deep in the mountains where there aren’t many people or Pokémon.");

    suggestions.push_back("It fires off ultrasonic waves from its red orbs to weaken its prey, and then it wraps them up in its 80 tentacles.It places small Pokémon into its spacious beak and carries them around. No one knows where it’s taking them.In many places, there are folktales of stardust falling into the ocean and becoming Staryu.");

    suggestions.push_back("Entei embodies the passion of magma. This Pokémon is thought to have been born in the eruption of a volcano. It sends up massive bursts of fire that utterly consume all that they touch.Pidgey has an extremely sharp sense of direction. It is capable of unerringly returning home to its nest, however far it may be removed from its familiar surroundings.With its two brains, it fires powerful psychic energy to stop its prey in their tracks.");

    suggestions.push_back("Bayleef’s neck is ringed by curled-up leaves. Inside each tubular leaf is a small shoot of a tree. The fragrance of this shoot makes people peppy.Krabby live on beaches, burrowed inside holes dug into the sand. On sandy beaches with little in the way of food, these Pokémon can be seen squabbling with each other over territory.Azurill’s tail is large and bouncy. It is packed full of the nutrients this Pokémon needs to grow. Azurill can be seen bouncing and playing on its big, rubbery tail.");

    suggestions.push_back("Whismur is very timid. If it starts to cry loudly, it becomes startled by its own crying and cries even harder. When it finally stops crying, the Pokémon goes to sleep, all tired out.The pink of Corsola that live in Alola is deep and vibrant, thanks to seas filled with nutrition.Weezing alternately shrinks and inflates its twin bodies to mix together toxic gases inside. The more the gases are mixed, the more powerful the toxins become. The Pokémon also becomes more putrid.");

    suggestions.push_back("Kyogre is said to be the personification of the sea itself. Legends tell of its many clashes against Groudon, as each sought to gain the power of nature.Zapdos is a legendary bird Pokémon that has the ability to control electricity. It usually lives in thunderclouds. The Pokémon gains power if it is stricken by lightning bolts.They’re popular, but they’re rare. Trainers who show them off recklessly may be targeted by thieves.");

    suggestions.push_back("Swellow is very conscientious about the upkeep of its glossy wings. Once two Swellow are gathered, they diligently take care of cleaning each other’s wings.Whenever it sees bird Pokémon flying through the sky, it becomes envious and smashes its surroundings to bits with headbutts.It lives in areas of limited rainfall. When danger approaches, it curls up into a ball to protect its soft stomach.");

    suggestions.push_back("Azurill’s tail is large and bouncy. It is packed full of the nutrients this Pokémon needs to grow. Azurill can be seen bouncing and playing on its big, rubbery tail.You can hear tales told all over the world about how Gengar will pay a visit to children who are naughty.Bulbasaur can be seen napping in bright sunlight. There is a seed on its back. By soaking up the sun’s rays, the seed grows progressively larger.");

    suggestions.push_back("Every once in a while, you’ll see a Golbat that’s missing some fangs. This happens when hunger drives it to try biting a Steel-type Pokémon.People believe that carrying one of its discarded fangs will prevent mishaps at sea, so the fangs are made into accessories.Charmeleon mercilessly destroys its foes using its sharp claws. If it encounters a strong foe, it turns aggressive. In this excited state, the flame at the tip of its tail flares with a bluish white color.");

    suggestions.push_back("Azurill’s tail is large and bouncy. It is packed full of the nutrients this Pokémon needs to grow. Azurill can be seen bouncing and playing on its big, rubbery tail.When it encounters an enemy that’s truly mighty, this Pokémon removes the power-save belt from its waist and unleashes its full power.Its body is like steel. Its tough, heavy pincers are more suited to smashing enemies than grabbing them.");

    suggestions.push_back("The fragrance of Meganium’s flower soothes and calms emotions. In battle, this Pokémon gives off more of its becalming scent to blunt the foe’s fighting spirit.People believe that carrying one of its discarded fangs will prevent mishaps at sea, so the fangs are made into accessories.Their food sources have decreased, and their numbers have declined sharply. Sludge ponds are being built to prevent their extinction.");

    suggestions.push_back("Swampert predicts storms by sensing subtle differences in the sounds of waves and tidal winds with its fins. If a storm is approaching, it piles up boulders to protect itself.Doduo’s two heads contain completely identical brains. A scientific study reported that on rare occasions, there will be examples of this Pokémon possessing different sets of brains.Overjoyed at finally being able to fly, it flies all over the place and usually doesn’t land until it’s completely exhausted and needs to sleep.");

    suggestions.push_back("When something approaches it, it fires off fragments of its steel shell in attack. This is not a conscious action but a conditioned reflex.As electricity builds up inside its body, it becomes more aggressive. One theory is that the electricity buildup is actually causing stress.Togetic is said to be a Pokémon that brings good fortune. When the Pokémon spots someone who is pure of heart, it is said to appear and share its happiness with that person.");
}