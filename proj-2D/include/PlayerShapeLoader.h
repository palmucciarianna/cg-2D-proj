// ============================================================================
// GUIDA RAPIDA PER L'ORALE - MODULO: PlayerShapeLoader (Parser Wavefront OBJ/MTL)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - È il parser che carica da disco i modelli poligonali in formato Wavefront .obj e le palette .mtl.
//    - Decodifica i token testuali:
//      * 'v x y z': estrae le posizioni cartesiane dei vertici;
//      * 'f v1 v2 v3 ... vn': triangola le facce poligonali con un ventaglio di triangoli (Triangle Fan);
//      * 'mtllib' e 'usemtl': collega i materiali del file .mtl;
//      * 'Kd r g b' e 'd alpha': estrae il colore diffuso e lo assegna direttamente come Vertex Color.
//    - Riempie la struttura MeshData pronta per il caricamento su VRAM tramite Geometry::upload().
//
// 2. A CHI È COLLEGATO:
//    - Invocato da: HermiteManager (per caricare 'alien_drone.obj' e 'warp_gate.obj') e Player.
//    - Produce: istanze della struttura MeshData.
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Triangolazione di Poligoni Arbitrari (Domande 2, 14):
//      OpenGL Core Profile accetta solo triangoli (GL_TRIANGLES). Per una faccia a N vertici:
//      (v0, v1, v2, ..., vn-1), il loader genera (N - 2) triangoli: (v0, v1, v2), (v0, v2, v3), ..., (v0, vn-2, vn-1).
//    - Flat Shading per Vertice (Domande 3, 7, 20):
//      Il parametro Kd del file MTL viene convertito direttamente in colore associato a ogni vertice (vec4 aColor).
//      Non essendoci luci 3D complesse (Phong/Blinn-Phong), il rendering è estremamente rapido.
// ============================================================================

#pragma once

#include "MeshData.h"
#include "OpenGL_Headers.h"
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

        std::vector<glm::vec3> vertices;                    // Lista temporanea dei vertici 'v' letti dal file
        std::unordered_map<std::string, glm::vec4> materials; // Mappa nome materiale -> colore RGBA
        // Colore di fallback azzurro chiaro se nessun materiale è associato.
        glm::vec4 currentColor(0.3f, 0.7f, 0.95f, 1.0f);

        MeshData result;                     // Struttura CPU di destinazione
        result.primitiveType = GL_TRIANGLES; // Tutti i poligoni vengono triangolati

        std::string line;                    // Buffer stringa per la lettura riga per riga

        // 2. Parsing riga per riga del file OBJ.
        while (std::getline(file, line))
        {
            std::istringstream in(line); // Stream di parsing sulla riga corrente
            std::string tag;             // Primo token della riga ('v', 'f', 'mtllib', 'usemtl')
            in >> tag;                   // Estrae il prefisso identificativo

            // Token 'v': vertice geometrico nello spazio locale (x, y, z).
            if (tag == "v")
            {
                glm::vec3 v(0.0f); // Inizializza il vertice a (0,0,0)
                if (in >> v.x >> v.y) // Legge almeno le coordinate X e Y
                {
                    in >> v.z; // Se z non è presente nello stream, rimane 0.0f
                    vertices.push_back(v); // Salva il vertice nella tabella indicizzata
                }
            }
            // Token 'mtllib': riferimento a un file di materiali esterno (.mtl).
            else if (tag == "mtllib")
            {
                std::string name; // Nome del file .mtl
                while (in >> name) // Carica ciascuna libreria materiali referenziata
                {
                    readMTL(directoryOf(filename) + name, materials); // Parsing colori dal file .mtl
                }
            }
            // Token 'usemtl': seleziona il materiale attivo per le facce successive.
            else if (tag == "usemtl")
            {
                std::string name; // Nome del materiale da attivare
                in >> name;       // Legge il nome dallo stream
                auto it = materials.find(name); // Cerca il materiale nella tabella caricata
                currentColor = (it != materials.end()) ? it->second : glm::vec4(0.3f, 0.7f, 0.95f, 1.0f); // Applica o usa fallback
            }
            // Token 'f': definizione di una faccia poligonale (es. f v1/vt1/vn1 v2/vt2/vn2 ...).
            else if (tag == "f")
            {
                std::vector<int> face; // Indici dei vertici appartenenti al poligono corrente
                std::string token;     // Singolo token indice (es. "12/3/1")

                // Estrae gli indici dei vertici della faccia.
                while (in >> token)
                {
                    try
                    {
                        // Estrae solo la prima cifra prima del carattere '/'.
                        int index = std::stoi(token.substr(0, token.find('/')));
                        if (index == 0) { face.clear(); break; } // Indice 0 non valido nello standard OBJ

                        // L'OBJ usa indici a base 1 (positivi) o relativi (negativi):
                        // Convertiamo all'indice base 0 del vettore C++.
                        index = (index > 0) ? (index - 1) : static_cast<int>(vertices.size()) + index;

                        // Verifica dei limiti di sicurezza del vettore.
                        if (index < 0 || index >= static_cast<int>(vertices.size()))
                        {
                            face.clear(); // Scarta la faccia se fuori intervallo
                            break;        // Interrompe la lettura della faccia
                        }
                        face.push_back(index); // Aggiunge l'indice validato
                    }
                    catch (...) // Cattura errori di conversione numerica (std::stoi)
                    {
                        face.clear(); // Scarta faccia malformata
                        break;        // Esce dal ciclo token
                    }
                }

                // 3. Triangolazione poligonale a ventaglio (Triangle Fan Triangulation):
                // Per un poligono con vertici v0, v1, v2, ..., v_n, genera i triangoli:
                // (v0, v1, v2), (v0, v2, v3), ..., (v0, v_{k}, v_{k+1})
                for (std::size_t k = 1; k + 1 < face.size(); ++k)
                {
                    const glm::vec3 a = vertices[face[0]];     // Primo vertice ancora (v0)
                    const glm::vec3 b = vertices[face[k]];     // Secondo vertice corrente (vk)
                    const glm::vec3 c = vertices[face[k + 1]]; // Terzo vertice successivo (vk+1)

                    // Calcolo del prodotto vettoriale 2D (area con segno):
                    // Se l'area è infinitesima, scarta il triangolo degenere per evitare artefatti di rasterizzazione.
                    const float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
                    if (std::abs(cross) < 1e-10f) continue; // Salta triangoli con area nulla

                    // Inserisce i tre vertici del triangolo nel buffer delle posizioni.
                    result.positions.push_back(a); // Vertice A
                    result.positions.push_back(b); // Vertice B
                    result.positions.push_back(c); // Vertice C

                    // Associa il colore del materiale attivo a tutti e tre i vertici del triangolo.
                    result.colors.insert(result.colors.end(), 3, currentColor);
                }
            }
        }

        // Verifica che il modello contenga almeno un triangolo valido.
        if (result.positions.empty())
        {
            error = "Il file OBJ non contiene facce poligonali o triangoli validi: " + filename; // Messaggio diagnostico
            return false; // Fallimento caricamento
        }

        // Trasferisce i risultati nella mesh di output tramite move semantics.
        mesh = std::move(result); // Evita copie pesanti dei std::vector
        error.clear();            // Nessun errore
        return true;              // Caricamento completato con successo
    }
}
