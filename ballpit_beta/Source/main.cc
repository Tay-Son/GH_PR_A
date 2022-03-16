#include "ParticleRenderer.h"

GS::ParticleRenderer* GS::ParticleRenderer::pr = 0;

int main() {
	GS::ParticleRenderer* pr = new GS::ParticleRenderer();
	//pr->RunBenchmark(pr);
	pr->Run(pr);
	delete pr;
	return 0;
}