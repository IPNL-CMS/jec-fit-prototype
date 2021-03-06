#!/usr/bin/env python

"""Fits L3Res jet correction.

Currently only the multijet analysis is supported.
"""

import argparse
import json

import jecfit


if __name__ == '__main__':

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument(
        '--multijet', help='File with inputs from multijet analysis'
    )
    arg_parser.add_argument(
        '-p', '--period', default='', help='Label for data-taking period'
    )
    arg_parser.add_argument(
        '-m', '--method', default='PtBal', help='Computation method'
    )
    arg_parser.add_argument(
        '--corr', default='2p', help='Functional form for jet correction'
    )
    arg_parser.add_argument(
        '--constraint',
        help='Constraint for jet correction of the form '
        '[<reference pt>,]<correction value>,<rel. uncertainty>'
    )
    arg_parser.add_argument(
        '-o', '--output', default='fit.json',
        help='Name for output JSON file'
    )
    arg_parser.add_argument(
        '-v', '--verbosity', type=int, default=3,
        help='Verbosity level to be used in the fit'
    )
    args = arg_parser.parse_args()
    
    if not args.multijet:
        raise RuntimeError('No inputs provided.')
    
    
    loss_func = jecfit.MultijetChi2(
        args.multijet, args.method, corr_form=args.corr,
        constraint_option=args.constraint
    )
    loss_func.set_pt_range(0., 1.6e3)
    fit_results = loss_func.fit(args.verbosity)


    results_to_store = fit_results.serialize()
    results_to_store.update({
        'ndf': loss_func.ndf,
        'p_value': loss_func.p_value(fit_results.min_value),
        'period': args.period,
        'variant': args.method,
        'corr_form': args.corr,
        'constraint': args.constraint
    })

    with open(args.output, 'w') as out_file:
        json.dump(results_to_store, out_file, indent=2)

