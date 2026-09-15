<?php

use Illuminate\Http\Request;
use Illuminate\Support\Facades\Route;
use App\Http\Controllers\Api\RollCallController;

Route::prefix('rollcall')->group(function () {
    Route::post('/store',  [RollCallController::class, 'store']);   // ESP -> يرسل JSON
    Route::get('/status',  [RollCallController::class, 'status']);  // Dashboard -> يسحب
});

Route::get('/ping', function () {
    return response()->json([
        'status' => 'ok',
        'time' => now()
    ]);
});