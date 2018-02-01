#include "GenFitExtrapolator.h"

GenFitExtrapolator::GenFitExtrapolator() {
}

GenFitExtrapolator::~GenFitExtrapolator() {
}

bool GenFitExtrapolator::init(std::string geometrySchema, double fMagStr,
		double kMagStr) {

	return true;
}

void GenFitExtrapolator::setInitialStateWithCov(double z_in, TMatrixD& state_in,
		TMatrixD& cov_in) {
}

void GenFitExtrapolator::setParticleType(int type) {
}

void GenFitExtrapolator::getFinalStateWithCov(TMatrixD& state_out,
		TMatrixD& cov_out) {
}

void GenFitExtrapolator::getPropagator(TMatrixD& prop) {
}

bool GenFitExtrapolator::extrapolateTo(double z_out) {

	return true;
}

int GenFitExtrapolator::propagate() {

	return -1;
}

double GenFitExtrapolator::extrapolateToIP() {

	return -1;
}

void GenFitExtrapolator::print() {
}
