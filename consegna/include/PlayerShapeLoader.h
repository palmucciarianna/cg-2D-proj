// ============================================================================
// File: include/PlayerShapeLoader.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Parser e caricatore procedurale di file Wavefront OBJ e tavolozze colore MTL.
//              Decodifica geometrie poligonali, estrae coordinate dei vertici,
//              associa colori flat RGB di vertice da file MTL (senza modelli di illuminazione)
//              e triangola poligoni arbitrari.
// ============================================================================

#pragma once

#include "MeshData.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace PlayerShapeLoader
{
    // Estrae la cartella genitore da un percorso di file completo.
    inline std::string directoryOf(const std::string& path)
    {
        const auto pos = path.find_last_of("/\\");
        return (pos == std::string::npos) ? std::string() : path.substr(0, pos + 1);
    }

    // ========================================================================
    // Lettura e Parsing dei Colori da File Wavefront MTL
    // ========================================================================
    // Decodifica i token 'newmtl' (nome colore/materiale), 'Kd' (colore base RGB)
    // e 'd' o 'Tr' (trasparenza/alpha) per colorare i vertici 2D (Vertex Color).
    // NOTA: Nessun modello di illuminazione (Phong/Blinn-Phong/Lambert) o calcolo
    // di normali: il valore Kd è utilizzato unicamente come colore flat per i vertici.
    inline bool readMTL(const std::string& filename, std::unordered_map<std::string, glm::vec4>& materials)
    {
        std::ifstream file(filename);
        if (!file) return false;

        std::string line, currentMaterial;

        // Itera su tutte le linee del file di materiale MTL.
        while (std::getline(file, line))
        {
            std::istringstream in(line);
            std::string tag;
            in >> tag;

            // Inizio definizione di un nuovo materiale.
            if (tag == "newmtl")
            {
                in >> currentMaterial;
                // Imposta un colore di default bianco opaco.
                materials[currentMaterial] = glm::vec4(1.0f);
            }
            // Canale del colore base RGB (Kd r g b in [0, 1] mappato su Vertex Color).
            else if (tag == "Kd" && !currentMaterial.empty())
            {
                float r, g, b;
                if (in >> r >> g >> b)
                {
                    materials[currentMaterial] = glm::vec4(r, g, b, 1.0f);
                }
            }
            // Canale di opacità/dissolve (d alpha).
            else if (tag == "d" && !currentMaterial.empty())
            {
                float a;
                if (in >> a)
                {
                    materials[currentMaterial].a = a;
                }
            }
        }

        return true;
    }

    // ========================================================================
    // Caricamento del Modello Geometrico Wavefront OBJ
    // ========================================================================
    inline bool load(const std::string& filename, MeshData& mesh, std::string& error)
    {
        // 1. Apertura del file OBJ su disco.
        std::ifstream file(filename);
        if (!file)
        {
            error = "Impossibile aprire il file OBJ: " + filename;
            return false;
        }

        std::vector<glm::vec3> vertices;
        std::unordered_map<std::string, glm::vec4> materials;
        // Colore di fallback azzurro chiaro se nessun materiale è associato.
        glm::vec4 currentColor(0.3f, 0.7f, 0.95f, 1.0f);

        MeshData result;
        result.primitiveType = GL_TRIANGLES;

        std::string line;

        // 2. Parsing riga per riga del file OBJ.
        while (std::getline(file, line))
        {
            std::istringstream in(line);
            std::string tag;
            in >> tag;

            // Token 'v': vertice geometrico nello spazio locale (x, y, z).
            if (tag == "v")
            {
                glm::vec3 v(0.0f);
                if (in >> v.x >> v.y)
                {
                    in >> v.z; // Se z non è presente nello stream, rimane 0.0f
                    vertices.push_back(v);
                }
            }
            // Token 'mtllib': riferimento a un file di materiali esterno (.mtl).
            else if (tag == "mtllib")
            {
                std::string name;
                while (in >> name)
                {
                    readMTL(directoryOf(filename) + name, materials);
                }
            }
            // Token 'usemtl': seleziona il materiale attivo per le facce successive.
            else if (tag == "usemtl")
            {
                std::string name;
                in >> name;
                auto it = materials.find(name);
                currentColor = (it != materials.end()) ? it->second : glm::vec4(0.3f, 0.7f, 0.95f, 1.0f);
            }
            // Token 'f': definizione di una faccia poligonale (es. f v1/vt1/vn1 v2/vt2/vn2 ...).
            else if (tag == "f")
            {
                std::vector<int> face;
                std::string token;

                // Estrae gli indici dei vertici della faccia.
                while (in >> token)
                {
                    try
                    {
                        // Estrae solo la prima cifra prima del carattere '/'.
                        int index = std::stoi(token.substr(0, token.find('/')));
                        if (index == 0) { face.clear(); break; }

                        // L'OBJ usa indici a base 1 (positivi) o relativi (negativi):
                        // Convertiamo all'indice base 0 del vettore C++.
                        index = (index > 0) ? (index - 1) : static_cast<int>(vertices.size()) + index;

                        // Verifica dei limiti di sicurezza del vettore.
                        if (index < 0 || index >= static_cast<int>(vertices.size()))
                        {
                            face.clear();
                            break;
                        }
                        face.push_back(index);
                    }
                    catch (...)
                    {
                        face.clear();
                        break;
                    }
                }

                // 3. Triangolazione poligonale a ventaglio (Triangle Fan Triangulation):
                // Per un poligono con vertici v0, v1, v2, ..., v_n, genera i triangoli:
                // (v0, v1, v2), (v0, v2, v3), ..., (v0, v_{k}, v_{k+1})
                for (std::size_t k = 1; k + 1 < face.size(); ++k)
                {
                    const glm::vec3 a = vertices[face[0]];
                    const glm::vec3 b = vertices[face[k]];
                    const glm::vec3 c = vertices[face[k + 1]];

                    // Calcolo del prodotto vettoriale 2D (area con segno):
                    // Se l'area è infinitesima, scarta il triangolo degenere per evitare artefatti di rasterizzazione.
                    const float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
                    if (std::abs(cross) < 1e-10f) continue;

                    // Inserisce i tre vertici del triangolo nel buffer delle posizioni.
                    result.positions.push_back(a);
                    result.positions.push_back(b);
                    result.positions.push_back(c);

                    // Associa il colore del materiale attivo a tutti e tre i vertici del triangolo.
                    result.colors.insert(result.colors.end(), 3, currentColor);
                }
            }
        }

        // Verifica che il modello contenga almeno un triangolo valido.
        if (result.positions.empty())
        {
            error = "Il file OBJ non contiene facce poligonali o triangoli validi: " + filename;
            return false;
        }

        // Trasferisce i risultati nella mesh di output tramite move semantics.
        mesh = std::move(result);
        error.clear();
        return true;
    }
}
