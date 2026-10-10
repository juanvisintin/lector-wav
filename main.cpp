#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <iomanip>

struct WavData {
	uint16_t audioFormat = 0;	// 1 = PCM sin compresión
	uint16_t numChannels = 0;	// 1 = mono ; 2 = estéreo
	uint32_t sampleRate = 0;	// ej: 44100 o 48000 Hz
	uint16_t bitsPerSample = 0;	// Ej: 16 bits
	std::vector<int16_t> samples;	// Muestras de audio intercaladas (L R L R...)

	// cantidad total de frames (muestras por canal)
	uint64_t numFrames() const {
		return numChannels ? samples.size() / numChannels : 0;

	}

	// duración total del audio en segundos
	double durationSeconds() const {
		return sampleRate ? static_cast<double>(numFrames()) / sampleRate : 0.0;
	}
};

// Lee un entero de 16 bits sin signo en Little-Endian
uint16_t readU16(std::istream& in) {
	unsigned char b[2];
	in.read(reinterpret_cast<char*>(b), 2);

	if (!in) throw std::runtime_error("Fin de archivo inesperado");
	
	return static_cast<uint16_t>(b[0] | b[1] << 8);
}

uint32_t readU32(std::istream& in) {
	unsigned char b[4];
	in.read(reinterpret_cast<char*>(b), 4);

	if (!in) throw std::runtime_error("Fin de archivo inesperado");

	return static_cast<uint32_t>(b[0] | (static_cast<uint32_t>(b[1]) << 8) |
		static_cast<uint32_t>(b[2]) << 16 | (static_cast<uint32_t>(b[3]) << 24));
}

// Lee una etiqueta de 4 caracteres (ej: "RIFF", "WAVE", "fmt ", "data")
std::string readTag(std::istream& in) {
	char t[4];
	in.read(t, 4);
	
	if (!in) return "";

	return std::string(t, 4);
}


WavData readWav(const std::string& path) {
	
	std::ifstream in(path, std::ios::binary);
	
	if (!in) {
		throw std::runtime_error("No se pudo abrir el archivo " + path);
	}

	// 1. Validar la cabecera RIFF / WAVE
	if (readTag(in) != "RIFF") throw std::runtime_error("No es un archivo RIFF");
	readU32(in); // tamaño total (no lo necesitamos)
	if (readTag(in) != "WAVE") throw std::runtime_error("No es un archivo WAVE");

	WavData wav;
	bool tieneFmt = false, tieneData = false;

	// 2. Recorrer los chunks hasta encontrar "fmt " y "data"
	// No asumimos que el encabezado mide 44 bytes: muchos WAV traen chunks extra (LIST, etc)
	while (!(tieneFmt && tieneData)) {
		std::string id = readTag(in);
		if (id.empty()) break; // fin del archivo
		uint32_t size = readU32(in);

		if (id == "fmt ") {
			wav.audioFormat = readU16(in);
			wav.numChannels = readU16(in);
			wav.sampleRate = readU32(in);
			in.seekg(6, std::ios::cur); // saltea los campos byteRate y blockAlign
			// readU32(in);
			// readU16(in);
			wav.bitsPerSample = readU16(in);

			if (size > 16) in.seekg(size - 16, std::ios::cur); // campos extra del fmt
			tieneFmt = true;
		}
		else if (id == "data") {
			
			// validaciones de seguridad:
			if (!tieneFmt) throw std::runtime_error("El chunk 'data' aparece antes que 'fmt '");
			if (wav.audioFormat != 1 || wav.bitsPerSample != 16)
				throw std::runtime_error("Solo se soporta PCM de 16 bits");

			// define un vector de elementos unsigned char, para guardar "size" elementos
			std::vector<unsigned char> raw(size);

			// lee todo el contenido del chunk "data"
			in.read(reinterpret_cast<char*>(raw.data()), size);

			if (!in) throw std::runtime_error("El chunk 'data' está truncado");

			// Define el tamaño del vector samples, de la estructura wav
			wav.samples.resize(size / 2);

			for (size_t i = 0; i < wav.samples.size(); ++i) {
				uint16_t u = static_cast<uint16_t>(raw[2 * i] | (raw[2 * i + 1] << 8));
				wav.samples[i] = static_cast<int16_t>(u); // convierte el uint a int
			}
			tieneData = true;
			if (size & 1) in.seekg(1, std::ios::cur);
		}
		else {
			// Chunk desconocido: lo saltamos (los chunks se alinean a bytes pares).
			in.seekg(size + (size & 1), std::ios::cur);
		}
	}

	if (!tieneFmt || !tieneData) throw std::runtime_error("Faltan chunks 'fmt ' o 'data'");
	return wav;

}

int main() {
	std::string ruta;

	std::cout << "\n\nDatos de un .wav\n================\n";
	std::cout << "Ruta del archivo: ";
	std::getline(std::cin, ruta);

	try {
		WavData wav = readWav(ruta);

		std::cout << "Archivo:			" << ruta << "\n"
			<< "Canales:			" << wav.numChannels << "\n"
			<< "Frecuencia de muestreo:	" << wav.sampleRate << "Hz\n"
			<< "Bits por muestra:	" << wav.bitsPerSample << "\n"
			<< "Frames:				" << wav.numFrames() << "\n"
			<< "Duración:			" << std::fixed << std::setprecision(2)
			<< wav.durationSeconds() << "seg\n";
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}
	return 0;
}